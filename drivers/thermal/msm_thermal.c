/* Author: Christopher R. Palmer <crpalmer@gmail.com>
 *
 * Very loosely based on a version released by HTC that was
 * Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/msm_tsens.h>
#include <linux/workqueue.h>
#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/msm_tsens.h>
#include <linux/msm_thermal.h>
#include <mach/cpufreq.h>
#include <mach/perflock.h>
#include <linux/earlysuspend.h>

#define NO_RELEASE_TEMPERATURE	0
#define NO_TRIGGER_TEMPERATURE	-1

#define N_TEMP_LIMITS	4

static unsigned temp_hysteresis = 5;
static unsigned int limit_temp_degC[N_TEMP_LIMITS] = { 63, 68, 75, 85 };
static unsigned int limit_freq[N_TEMP_LIMITS] = { 1728000, 1350000, 918000, 384000 };

module_param_array(limit_temp_degC, uint, NULL, 0644);
module_param_array(limit_freq, uint, NULL, 0644);

static int throttled_bin = -1;

module_param(throttled_bin, int, 0444);

static struct msm_thermal_data msm_thermal_info;
static struct delayed_work first_work;
static struct work_struct trip_work;


/*static void check_max_freq(void)
{

	if (limit_freq[0] > arg_max_oc0) {
		limit_freq[0] = arg_max_oc0;
		pr_info("msm_thermal: changing limit 1 to %d\n", limit_freq[0]);
	}

	if (limit_freq[1] > arg_max_oc0) {
		limit_freq[1] = arg_max_oc0;
		pr_info("msm_thermal: changing limit 2 to %d\n", limit_freq[1]);
	}
}*/

static int max_freq(int throttled_bin)
{
	if (throttled_bin < 0) return MSM_CPUFREQ_NO_LIMIT;
	else return limit_freq[throttled_bin];
}

static int limit_temp(int throttled_bin)
{
	if (throttled_bin < 0) return limit_temp_degC[0];
	else if (throttled_bin == N_TEMP_LIMITS-1) return NO_TRIGGER_TEMPERATURE;
	else return limit_temp_degC[throttled_bin+1];
}

static int release_temp(int throttled_bin)
{
	if (throttled_bin < 0) return NO_RELEASE_TEMPERATURE;
	else return limit_temp_degC[throttled_bin] - temp_hysteresis;
}
	
static int update_cpu_max_freq(int cpu, int throttled_bin, unsigned temp)
{
	int ret;
	int max_frequency = max_freq(throttled_bin);

	ret = msm_cpufreq_set_freq_limits(cpu, MSM_CPUFREQ_NO_LIMIT, max_frequency);
	if (ret)
		return ret;

	ret = cpufreq_update_policy(cpu);
	if (ret)
		return ret;

	if (max_frequency != MSM_CPUFREQ_NO_LIMIT) {
		struct cpufreq_policy policy;

		if ((ret = cpufreq_get_policy(&policy, cpu)) == 0)
			ret = cpufreq_driver_target(&policy, max_frequency, CPUFREQ_RELATION_L);
	}

	if (max_frequency != MSM_CPUFREQ_NO_LIMIT)
		pr_info("msm_thermal: limiting cpu%d max frequency to %d at %u degC\n",
				cpu, max_frequency, temp);
	else
		pr_info("msm_thermal: Max frequency reset for cpu%d at %u degC\n", cpu, temp);

	return ret;
}

static void
update_all_cpus_max_freq_if_changed(int new_throttled_bin, unsigned temp)
{
	int cpu;
	int ret;

	if (throttled_bin == new_throttled_bin)
		return;

#ifdef CONFIG_PERFLOCK_BOOT_LOCK
	release_boot_lock();
#endif

	throttled_bin = new_throttled_bin;
	

	for_each_possible_cpu(cpu) {
		ret = update_cpu_max_freq(cpu, throttled_bin, temp);
		if (ret)
			pr_warn("Unable to limit cpu%d\n", cpu);
	}
}

static void
configure_sensor_trip_points(void)
{
	int trigger_temperature = limit_temp(throttled_bin);
	int release_temperature = release_temp(throttled_bin);

	pr_info("msm_thermal: setting trip range %d..%d on sensor %d.\n", release_temperature, 			trigger_temperature, msm_thermal_info.sensor_id); 
	if (trigger_temperature != NO_TRIGGER_TEMPERATURE)
		tsens_set_tz_warm_temp_degC(msm_thermal_info.sensor_id, trigger_temperature, &trip_work);

	if (release_temperature != NO_RELEASE_TEMPERATURE)
		tsens_set_tz_cool_temp_degC(msm_thermal_info.sensor_id, release_temperature, &trip_work);
}

static int
select_throttled_bin(unsigned temp)
{
	int i;
	int new_bin = -1;

	for (i = 0; i < N_TEMP_LIMITS; i++) {
		if (temp >= limit_temp_degC[i]) new_bin = i;
	}

	if (new_bin > throttled_bin) return new_bin;
	if (temp <= release_temp(throttled_bin)) return new_bin;
	return throttled_bin;
}

static void check_temp_and_throttle_if_needed(struct work_struct *work)
{
	struct tsens_device tsens_dev;
	unsigned long temp_ul = 0;
	unsigned temp;
	int new_bin;
	int ret;

	tsens_dev.sensor_num = msm_thermal_info.sensor_id;
	ret = tsens_get_temp(&tsens_dev, &temp_ul);
	if (ret) {
		pr_warn("msm_thermal: Unable to read TSENS sensor %d\n",
				tsens_dev.sensor_num);
		return;
	}

	temp = (unsigned) temp_ul;
	new_bin = select_throttled_bin(temp);

	pr_debug("msm_thermal: TSENS sensor %d is %u degC old-bin %d new-bin %d\n",
		tsens_dev.sensor_num, temp, throttled_bin, new_bin);
	update_all_cpus_max_freq_if_changed(new_bin, temp);
}

static void check_temp(struct work_struct *work)
{
	check_temp_and_throttle_if_needed(work);
	configure_sensor_trip_points();
}

int __init msm_thermal_init(struct msm_thermal_data *pdata)
{
	BUG_ON(!pdata);
	BUG_ON(pdata->sensor_id >= TSENS_MAX_SENSORS);
	memcpy(&msm_thermal_info, pdata, sizeof(struct msm_thermal_data));

//	check_max_freq();

	INIT_DELAYED_WORK(&first_work, check_temp);
	INIT_WORK(&trip_work, check_temp);

	schedule_delayed_work(&first_work, msecs_to_jiffies(5*1000)); 

	return 0;
}


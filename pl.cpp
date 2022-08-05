/* (C) 2006-2010 by folkert@vanheusden.com GPLv2 applies */

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <ctype.h>

extern "C" {
#include "error.h"
}
#include "utils.h"
#include "br.h"
#include "pl.h"

static struct v2_0_config v2_0_config_elements[] = {
	{"acknowledgement_type",	STATS_OFFSET(acknowledgement_type),	VARTYPE_INT}, 
	{"active_checks_enabled",	STATS_OFFSET(active_checks_enabled),	VARTYPE_INT}, 
	{"author",			STATS_OFFSET(author),			VARTYPE_PCHAR},
	{"check_command",		STATS_OFFSET(check_command),		VARTYPE_PCHAR}, 
	{"check_execution_time",	STATS_OFFSET(check_execution_time),	VARTYPE_DOUBLE}, 
	{"check_interval",		STATS_OFFSET(check_interval),		VARTYPE_DOUBLE},
	{"check_latency",		STATS_OFFSET(check_latency),		VARTYPE_DOUBLE}, 
	{"check_options",		STATS_OFFSET(check_options),		VARTYPE_INT},
	{"check_period",		STATS_OFFSET(check_period),		VARTYPE_PCHAR},
	{"check_type",			STATS_OFFSET(check_type),		VARTYPE_INT}, 
	{"comment",			STATS_OFFSET(comment),			VARTYPE_PCHAR},
	{"comment_data",		STATS_OFFSET(comment_data),		VARTYPE_PCHAR},
	{"comment_id",			STATS_OFFSET(comment_id),		VARTYPE_INT},
	{"current_attempt",		STATS_OFFSET(current_attempt),		VARTYPE_INT}, 
	{"current_event_id",		STATS_OFFSET(current_event_id),		VARTYPE_INT},
	{"current_notification_id",	STATS_OFFSET(current_notification_id),	VARTYPE_INT},
	{"current_notification_number",	STATS_OFFSET(current_notification_number),	VARTYPE_DOUBLE}, 
	{"current_problem_id",		STATS_OFFSET(current_problem_id),	VARTYPE_INT},
	{"current_state",		STATS_OFFSET(current_state),		VARTYPE_INT}, 
	{"downtime_id",			STATS_OFFSET(downtime_id),		VARTYPE_INT},
	{"duration",			STATS_OFFSET(duration),			VARTYPE_INT},
	{"end_time",			STATS_OFFSET(end_time),			VARTYPE_TIMET},
	{"entry_time",			STATS_OFFSET(entry_time),		VARTYPE_TIMET},
	{"entry_type",			STATS_OFFSET(entry_type),		VARTYPE_INT},
	{"event_handler",		STATS_OFFSET(event_handler),		VARTYPE_INT}, 
	{"event_handler_enabled",	STATS_OFFSET(event_handler_enabled),	VARTYPE_INT}, 
	{"expire_time",			STATS_OFFSET(expire_time),		VARTYPE_TIMET},
	{"expires",			STATS_OFFSET(expires),			VARTYPE_INT},
	{"failure_prediction_enabled",	STATS_OFFSET(failure_prediction_enabled),	VARTYPE_INT}, 
	{"fixed",			STATS_OFFSET(fixed),			VARTYPE_INT},
	{"flap_detection_enabled",	STATS_OFFSET(flap_detection_enabled),	VARTYPE_INT}, 
	{"has_been_checked",		STATS_OFFSET(has_been_checked),		VARTYPE_INT}, 
	{"host_name",			STATS_OFFSET(host_name),		VARTYPE_PCHAR}, 
	{"host_notification_period",	STATS_OFFSET(host_notification_period),	VARTYPE_PCHAR},
	{"is_flapping",			STATS_OFFSET(is_flapping),		VARTYPE_INT}, 
	{"last_check",			STATS_OFFSET(last_check),		VARTYPE_TIMET}, 
	{"last_event_id",		STATS_OFFSET(last_event_id),		VARTYPE_INT},
	{"last_hard_state",		STATS_OFFSET(last_hard_state),		VARTYPE_INT}, 
	{"last_hard_state_change",	STATS_OFFSET(last_hard_state_change),	VARTYPE_INT}, 
	{"last_notification",		STATS_OFFSET(last_notification),	VARTYPE_INT}, 
	{"last_problem_id",		STATS_OFFSET(last_problem_id),		VARTYPE_INT},
	{"last_state_change",		STATS_OFFSET(last_state_change),	VARTYPE_TIMET}, 
	{"last_time_critical",		STATS_OFFSET(last_time_critical),	VARTYPE_INT}, 
	{"last_time_down",		STATS_OFFSET(last_time_down),		VARTYPE_INT}, 
	{"last_time_ok",		STATS_OFFSET(last_time_ok),		VARTYPE_INT}, 
	{"last_time_unknown",		STATS_OFFSET(last_time_unknown),	VARTYPE_INT}, 
	{"last_time_unreachable",	STATS_OFFSET(last_time_unreachable),	VARTYPE_INT}, 
	{"last_time_up",		STATS_OFFSET(last_time_up),		VARTYPE_INT}, 
	{"last_time_warning",		STATS_OFFSET(last_time_warning),	VARTYPE_INT}, 
	{"last_update",			STATS_OFFSET(last_update),		VARTYPE_TIMET}, 
	{"long_plugin_output",		STATS_OFFSET(long_plugin_output),	VARTYPE_PCHAR},
	{"max_attempts",		STATS_OFFSET(max_attempts),		VARTYPE_INT}, 
	{"modified_attributes",		STATS_OFFSET(modified_attributes),	VARTYPE_INT}, 
	{"next_check",			STATS_OFFSET(next_check),		VARTYPE_TIMET}, 
	{"next_comment_id",		STATS_OFFSET(next_comment_id),		VARTYPE_INT},
	{"next_notification",		STATS_OFFSET(next_notification),	VARTYPE_INT}, 
	{"no_more_notifications",	STATS_OFFSET(no_more_notifications),	VARTYPE_INT}, 
	{"notification_period",		STATS_OFFSET(notification_period),	VARTYPE_PCHAR},
	{"notifications_enabled",	STATS_OFFSET(notifications_enabled),	VARTYPE_INT}, 
	{"obsess",		 	STATS_OFFSET(obsess),			VARTYPE_INT},
	{"obsess_over_host",		STATS_OFFSET(obsess_over_host),		VARTYPE_INT},
	{"obsess_over_service",		STATS_OFFSET(obsess_over_service),	VARTYPE_INT}, 
	{"passive_checks_enabled",	STATS_OFFSET(passive_checks_enabled),	VARTYPE_INT}, 
	{"percent_state_change",	STATS_OFFSET(percent_state_change),	VARTYPE_DOUBLE}, 
	{"performance_data",		STATS_OFFSET(performance_data),		VARTYPE_PCHAR}, 
	{"persistent",			STATS_OFFSET(persistent),		VARTYPE_INT},
	{"plugin_output",		STATS_OFFSET(plugin_output),		VARTYPE_PCHAR}, 
	{"problem_has_been_acknowledged",	STATS_OFFSET(problem_has_been_acknowledged),	VARTYPE_INT}, 
	{"process_performance_data",	STATS_OFFSET(process_performance_data),	VARTYPE_INT}, 
	{"retry_interval",		STATS_OFFSET(retry_interval),		VARTYPE_DOUBLE},
	{"scheduled_downtime_depth",	STATS_OFFSET(scheduled_downtime_depth),	VARTYPE_DOUBLE}, 
	{"service_description",		STATS_OFFSET(service_description),	VARTYPE_PCHAR}, 
	{"should_be_scheduled",		STATS_OFFSET(should_be_scheduled),	VARTYPE_INT}, 
	{"source",			STATS_OFFSET(source),			VARTYPE_INT},
	{"start_time",			STATS_OFFSET(start_time),		VARTYPE_TIMET},
	{"state_type",			STATS_OFFSET(state_type),		VARTYPE_INT}, 
	{"triggered_by",		STATS_OFFSET(triggered_by),		VARTYPE_INT},
	{"type",			STATS_OFFSET(type),			VARTYPE_INT}
};

/*
   service {
   host_name=ap
   service_description=HTTP
   modified_attributes=0
   check_command=check_tcp!80
   event_handler=
   has_been_checked=1
   should_be_scheduled=1
   check_execution_time=0.009
   check_latency=0.712
   current_state=0
   last_hard_state=0
   current_attempt=1
   max_attempts=3
   state_type=1
   last_state_change=1128634047
   last_hard_state_change=1128633828
   last_time_ok=1129101510
   last_time_warning=0
   last_time_unknown=1128633828
   last_time_critical=1126954624
   plugin_output=TCP OK - 0.001 second response time on port 80
   performance_data=time=0.001121s;0.000000;0.000000;0.000000;10.000000
   last_check=1129101510
   next_check=1129101630
   check_type=0
   current_notification_number=0
   last_notification=0
   next_notification=0
   no_more_notifications=0
   notifications_enabled=1
   active_checks_enabled=1
   passive_checks_enabled=1
   event_handler_enabled=1
   problem_has_been_acknowledged=0
   acknowledgement_type=0
   flap_detection_enabled=1
   failure_prediction_enabled=1
   process_performance_data=1
   obsess_over_service=1
   last_update=1129101556
   is_flapping=0
   percent_state_change=0.00
   scheduled_downtime_depth=0
   }

   host {
   host_name=ap
   modified_attributes=0
   check_command=check-host-alive
   event_handler=
   has_been_checked=1
   should_be_scheduled=0
   check_execution_time=0.012
   check_latency=0.000
   current_state=0
   last_hard_state=0
   check_type=0
   plugin_output=PING OK - Packet loss = 0%, RTA = 1.54 ms
   performance_data=
   last_check=1128863442
   next_check=0
   current_attempt=1
   max_attempts=10
   state_type=1
   last_state_change=1128634057
   last_hard_state_change=1128634057
   last_time_up=1128863442
   last_time_down=1128633868
   last_time_unreachable=0
   last_notification=1128634057
next_notification=0
no_more_notifications=0
current_notification_number=0
notifications_enabled=1
problem_has_been_acknowledged=0
acknowledgement_type=0
active_checks_enabled=1
passive_checks_enabled=1
event_handler_enabled=1
flap_detection_enabled=1
failure_prediction_enabled=1
process_performance_data=1
obsess_over_host=1
last_update=1129101556
is_flapping=0
percent_state_change=0.00
scheduled_downtime_depth=0
}
*/

int v2_0_find_entry_type(char *field_name)
{
	int left = 0;
	int right = (sizeof(v2_0_config_elements) / sizeof(struct v2_0_config)) - 1;

	while(left <= right)
	{
		int mid = (left + right) / 2;
		int compare = strcmp(field_name, v2_0_config_elements[mid].str);

		if (compare > 0)
			left = mid + 1;
		else if (compare < 0)
			right = mid - 1;
		else
			return mid;
	}

	return -1;
}

void parse_2_0_statuslog(int fd, struct stats **pstats, int *n_stats)
{
	int n_alloc = 0;
	int type = TYPE_IGNORE;

	/* start up buffered reader */
	buffered_reader bf(fd);

	*pstats = NULL;
	*n_stats = 0;

	for(;;)
	{
		char *line = bf.read_line();
		if (!line)
			break;

		if (strchr(line, '{'))
		{
			/* init */
			type = TYPE_IGNORE;

			if (*n_stats == n_alloc)
			{
				if (n_alloc)
					n_alloc *= 2;
				else
					n_alloc = 128;

				*pstats = (struct stats *)myrealloc(*pstats, n_alloc * sizeof(struct stats), "stats array");
				if (!*pstats)
					error_exit("Error allocating memory");

				memset(&(*pstats)[*n_stats], 0x00, sizeof(struct stats) * (n_alloc - *n_stats));
			}
			else
			{
				memset(&(*pstats)[*n_stats], 0x00, sizeof(struct stats));
			}


			if (strncmp(line, "host", 4) == 0)
			{
				type = TYPE_HOST;
			}
			else if (strncmp(line, "service", 7) == 0)
			{
				type = TYPE_SERVICE;
			}
			else
			{
				type = TYPE_IGNORE;
			}
		}
		else if (type != TYPE_IGNORE)
		{
			char *cmd = line, *is;
			while(*cmd == ' ' || *cmd == '\t') cmd++;

			is = strchr(cmd, '=');
			if (is == NULL)
			{
				if (strchr(cmd, '}') != NULL)	/* end of definition, store in array */
				{
					if (type == TYPE_HOST && (*pstats)[*n_stats].current_state == 1) /* HOSTS: 0=ok, 1=down, 2=unreachable */
						(*pstats)[*n_stats].current_state = 2;
					(*pstats)[*n_stats].type = type;
					(*n_stats)++;
				}
			}
			else
			{
				int index;
				char *record = (char *)(&(*pstats)[*n_stats]);
				char *par = is + 1;
				*is = 0x00;

				if ((index = v2_0_find_entry_type(cmd)) != -1)
				{
					switch(v2_0_config_elements[index].type)
					{
						case VARTYPE_PCHAR:
							*((char **)(&record[v2_0_config_elements[index].offset])) = mystrdup(par);
							break;
						case VARTYPE_INT:
							*((int *)(&record[v2_0_config_elements[index].offset])) = atoi(par);
							break;
						case VARTYPE_TIMET:
							*((time_t *)(&record[v2_0_config_elements[index].offset])) = atol(par);
							break;
						case VARTYPE_DOUBLE:
							*((double *)(&record[v2_0_config_elements[index].offset])) = atof(par);
							break;
					}
				}
				else
				{
					/*printf("{%s} ???\n", cmd);*/
				}
			}
		}

		/* this one does not use myfree() as the buffered reader does not
		 * use mymalloc() and friends
		 */
		free(line);
	}
}

int split_1_0_line(char *line, char **out)
{
	int out_index = 0;

	memset(out, 0x00, sizeof(char *) * V1_0_MAX_ELEMENTS);

	for(;out_index < V1_0_MAX_ELEMENTS;)
	{
		out[out_index++] = line;

		line = strchr(line, ';');

		if (!line)
			break;

		*line = 0x00; /* replace ';' with 0x00 */
		line++;
	}

	return out_index;
}

void parse_1_0_statuslog(int fd, struct stats **pstats, int *n_stats)
{
	/* start up buffered reader */
	buffered_reader bf(fd);

	*pstats = NULL;
	*n_stats = 0;

	for(;;)
	{
		int type = TYPE_IGNORE;
		char *host_name = NULL;
		int current_state = -1;
		char *service_description = NULL;
		char *plugin_output = NULL;
		time_t last_state_change = 0;
		char active_checks_enabled = 0;
		char passive_checks_enabled = 0;
		char notifications_enabled = 0;
		char problem_has_been_acknowledged = 0;
		int scheduled_downtime_depth = 0;
		char state_type = 0;
		char last_hard_state = 0;
		int modified_attributes = 0;
		int event_handler = 0;
		int has_been_checked = 0;
		int should_be_scheduled = 0;
		int current_attempt = 0;
		int max_attempts = 0;
		int last_hard_state_change = 0;
		int last_time_ok = 0;
		int last_time_warning = 0;
		int last_time_unknown = 0;
		int last_time_critical = 0;
		time_t last_check = 0;
		time_t next_check = 0;
		int check_type = 0;
		int current_notification_number = 0;
		int last_notification = 0;
		int next_notification = 0;
		int no_more_notifications = 0;
		int event_handler_enabled = 0;
		int acknowledgement_type = 0;
		int flap_detection_enabled = 0;
		int failure_prediction_enabled = 0;
		int process_performance_data = 0;
		int obsess_over_service = 0;
		time_t last_update = 0;
		int is_flapping = 0;
		double percent_state_change = 0;
		double check_execution_time = 0;
		double check_latency = 0;
		char * performance_data = NULL;
		char * check_command = NULL;
		int last_time_up = 0;
		int last_time_down = 0;
		int last_time_unreachable = 0;

		char *dummy;
		char *elements[V1_0_MAX_ELEMENTS];
		int n_elem;
		char *line = bf.read_line();
		if (!line)
			break;

		if (line[0] == '#')
			goto skip_line;

		n_elem = split_1_0_line(line, elements);

		*pstats = (struct stats *)myrealloc(*pstats, ((*n_stats) + 1) * sizeof(struct stats), "stats array");
		if (!*pstats)
			error_exit("Error allocating memory");

		memset(&(*pstats)[*n_stats], 0x00, sizeof(struct stats));

		/*
		   [Time of last update] HOST;
		   Host Name (string);
		   Status (OK/DOWN/UNREACHABLE);
		   Last Check Time (long time);
		   Last State Change (long time);
		   Acknowledged (0/1);
		   Time Up (long time);
		   Time Down (long time);
		   Time Unreachable (long time);
		   Last Notification Time (long time);
		   Current Notification Number (#);
		   Notifications Enabled (0/1);
		   Event Handlers Enabled (0/1);
		   Checks Enabled (0/1);
		   Flap Detection Enabled (0/1);
		   Host is Flapping (0/1);
		   Percent State Change (###.##);
		   Scheduled downtime depth (#);
		   Failure Prediction Enabled (0/1);
		   Process Performance Data(0/1);
		   Plugin Output (string)
		   */

		/*
		   [Time of last update] SERVICE;
		   Host Name (string);
		   Service Description (string);
		   Status (OK/WARNING/CRITICAL/UNKNOWN);
		   Retry number (#/#);
		   State Type (SOFT/HARD);
		   Last check time (long time);
		   Next check time (long time);
		   Check type (ACTIVE/PASSIVE);
		   Checks enabled (0/1);
		   Accept Passive Checks (0/1);
		   Event Handlers Enabled (0/1);
		   Last state change (long time);
		   Problem acknowledged (0/1);
		   Last Hard State (OK/WARNING/CRITICAL/UNKNOWN);
		   Time OK (long time);
		   Time Unknown (long time);
		   Time Warning (long time);
		   Time Critical (long time);
		   Last Notification Time (long time);
		   Current Notification Number (#);
		   Notifications Enabled (0/1);
		   Latency (#);
		   Execution Time (#);
		   Flap Detection Enabled (0/1);
		   Service is Flapping (0/1);
		   Percent State Change (###.##);
		   Scheduled Downtime Depth (#);
		   Failure Prediction Enabled (0/1);
		   Process Performance Date (0/1);
		   Obsess Over Service (0/1);
		   Plugin Output (string)
		   */

		dummy = strchr(elements[0], ' ');
		if (!dummy)
			goto skip_line;
		dummy++;
		if (strcmp(dummy, "HOST") == 0)
			type = TYPE_HOST;
		else if (strcmp(dummy, "SERVICE") == 0)
			type = TYPE_SERVICE;
		else if (strcmp(dummy, "PROGRAM") == 0)
			goto skip_line;
		else
			goto skip_line;

		last_update = atol(elements[0] + 1);

		if (type == TYPE_HOST)
		{
			if (n_elem != 21)
				goto skip_line;

			host_name = mystrdup(elements[1]);
			if (strcmp(elements[2], "UP") == 0)
				current_state = 0;
			else
				current_state = 2;
			last_check = atol(elements[3]);
			last_state_change = atol(elements[4]);
			problem_has_been_acknowledged = atoi(elements[5]);
			last_time_up = atol(elements[6]);
			last_time_down = atol(elements[7]);
			last_time_unreachable = atol(elements[8]);
			last_notification = atol(elements[9]);
			current_notification_number = atoi(elements[10]);
			notifications_enabled = atoi(elements[11]);
			event_handler_enabled = atoi(elements[12]);
			active_checks_enabled = atoi(elements[13]); /* in 2.0 it has been split up in passive and active */
			flap_detection_enabled = atoi(elements[14]);
			is_flapping = atoi(elements[15]);
			percent_state_change = atof(elements[16]);
			scheduled_downtime_depth = atoi(elements[17]);
			failure_prediction_enabled = atoi(elements[18]);
			/*			process_performance_data = atoi(elements[19]); */
			plugin_output = mystrdup(elements[20]);
		}
		else if (type == TYPE_SERVICE)
		{
			if (n_elem != 32)
				goto skip_line;
			host_name = mystrdup(elements[1]);
			service_description = mystrdup(elements[2]);
			if (strcmp(elements[3], "OK") == 0)
				current_state = 0;
			else if (strcmp(elements[3], "WARNING") == 0)
				current_state = 1;
			else if (strcmp(elements[3], "CRITICAL") == 0)
				current_state = 2;
			else if (strcmp(elements[3], "UNKNOWN") == 0 || strcmp(elements[3], "PENDING") == 0)
				current_state = 3;
			/*			retry_number = atoi(elements[4]); */
			if (strcmp(elements[5], "SOFT") == 0)
				state_type = 0;
			else
				state_type = 1;
			last_check = atol(elements[6]);
			next_check = atol(elements[7]);
			if (strcmp(elements[8], "ACTIVE") == 0)
				check_type = 1;
			else
				check_type = 0;
			active_checks_enabled = atoi(elements[9]);
			/*			accept_passive_checks = atoi(elements[10]); */
			event_handler_enabled = atoi(elements[11]);
			last_state_change = atol(elements[12]);
			problem_has_been_acknowledged = atoi(elements[13]);
			if (strcmp(elements[14], "OK") == 0)
				last_hard_state = 0;
			else if (strcmp(elements[14], "WARNING") == 0)
				last_hard_state = 1;
			else if (strcmp(elements[14], "CRITICAL") == 0)
				last_hard_state = 2;
			else if (strcmp(elements[14], "UNKNOWN") == 0)
				last_hard_state = 3;
			last_time_ok = atol(elements[15]);
			last_time_unknown = atol(elements[16]);
			last_time_warning = atol(elements[17]);
			last_time_critical = atol(elements[18]);
			last_notification = atol(elements[19]);
			current_notification_number = atoi(elements[20]);
			notifications_enabled = atoi(elements[21]);
			check_latency = atof(elements[22]);
			check_execution_time = atof(elements[23]);
			flap_detection_enabled = atoi(elements[24]);
			is_flapping = atoi(elements[25]);
			percent_state_change = atof(elements[26]);
			scheduled_downtime_depth = atoi(elements[27]);
			failure_prediction_enabled = atoi(elements[28]);
			/*			process_performance_date = atoi(elements[29]); */
			obsess_over_service = atoi(elements[30]);
			plugin_output = mystrdup(elements[31]);
		}
		else
			error_exit("internal error: type %d unexpected\n", type);

		(*pstats)[*n_stats].type = type;
		(*pstats)[*n_stats].host_name = host_name?host_name:mystrdup("");
		if (type == TYPE_HOST && current_state == 1) /* HOSTS: 0=ok, 1=down, 2=unreachable */
			current_state = 2;
		(*pstats)[*n_stats].current_state = current_state;
		(*pstats)[*n_stats].service_description = service_description?service_description:mystrdup("");
		(*pstats)[*n_stats].plugin_output = plugin_output?plugin_output:mystrdup("");
		(*pstats)[*n_stats].last_state_change = last_state_change;
		(*pstats)[*n_stats].active_checks_enabled = active_checks_enabled;
		(*pstats)[*n_stats].passive_checks_enabled = passive_checks_enabled;
		(*pstats)[*n_stats].notifications_enabled = notifications_enabled;
		(*pstats)[*n_stats].problem_has_been_acknowledged = problem_has_been_acknowledged;
		(*pstats)[*n_stats].scheduled_downtime_depth = scheduled_downtime_depth;
		(*pstats)[*n_stats].state_type = state_type;
		(*pstats)[*n_stats].last_hard_state = last_hard_state;
		(*pstats)[*n_stats].modified_attributes = modified_attributes;
		(*pstats)[*n_stats].event_handler = event_handler;
		(*pstats)[*n_stats].has_been_checked = has_been_checked;
		(*pstats)[*n_stats].should_be_scheduled = should_be_scheduled;
		(*pstats)[*n_stats].current_attempt = current_attempt;
		(*pstats)[*n_stats].max_attempts = max_attempts;
		(*pstats)[*n_stats].last_hard_state_change = last_hard_state_change;
		(*pstats)[*n_stats].last_time_ok = last_time_ok;
		(*pstats)[*n_stats].last_time_warning = last_time_warning;
		(*pstats)[*n_stats].last_time_unknown = last_time_unknown;
		(*pstats)[*n_stats].last_time_critical = last_time_critical;
		(*pstats)[*n_stats].last_check = last_check;
		(*pstats)[*n_stats].next_check = next_check;
		(*pstats)[*n_stats].check_type = check_type;
		(*pstats)[*n_stats].current_notification_number = current_notification_number;
		(*pstats)[*n_stats].last_notification = last_notification;
		(*pstats)[*n_stats].next_notification = next_notification;
		(*pstats)[*n_stats].no_more_notifications = no_more_notifications;
		(*pstats)[*n_stats].event_handler_enabled = event_handler_enabled;
		(*pstats)[*n_stats].acknowledgement_type = acknowledgement_type;
		(*pstats)[*n_stats].flap_detection_enabled = flap_detection_enabled;
		(*pstats)[*n_stats].failure_prediction_enabled = failure_prediction_enabled;
		(*pstats)[*n_stats].process_performance_data = process_performance_data;
		(*pstats)[*n_stats].obsess_over_service = obsess_over_service;
		(*pstats)[*n_stats].last_update = last_update;
		(*pstats)[*n_stats].is_flapping = is_flapping;
		(*pstats)[*n_stats].percent_state_change = percent_state_change;
		(*pstats)[*n_stats].check_execution_time = check_execution_time;
		(*pstats)[*n_stats].check_latency = check_latency;
		(*pstats)[*n_stats].performance_data = performance_data;
		(*pstats)[*n_stats].check_command = check_command;
		(*pstats)[*n_stats].last_time_up = last_time_up;
		(*pstats)[*n_stats].last_time_down = last_time_down;
		(*pstats)[*n_stats].last_time_unreachable = last_time_unreachable;
		(*n_stats)++;

skip_line:
		free(line);
	}

	close(fd);
}

void free_stats_array(struct stats *pstats, int n_stats)
{
	if (pstats)
	{
		for(int loop=0; loop<n_stats; loop++)
		{
			myfree((void *)pstats[loop].host_name, "entry hostname");
			myfree((void *)pstats[loop].service_description, "entry servicedescription");
			myfree((void *)pstats[loop].plugin_output, "entry plugin output");
			myfree((void *)pstats[loop].performance_data, "entry performance data");
			myfree((void *)pstats[loop].check_command, "entry check command");
			myfree((void *)pstats[loop].author, "entry author");
			myfree((void *)pstats[loop].check_period, "entry check_period");
			myfree((void *)pstats[loop].comment_data, "entry comment data");
			myfree((void *)pstats[loop].host_notification_period, "entry host notification period");
			myfree((void *)pstats[loop].long_plugin_output, "entry long plugin output");
			myfree((void *)pstats[loop].notification_period, "entry notification period");
			myfree((void *)pstats[loop].service_notification_period, "entry service notification period");
		}

		myfree((void *)pstats, "stats array");
	}
}

int sort_compare_func(const void *a, const void *b)
{
	struct stats *p1 = (struct stats *)a;
	struct stats *p2 = (struct stats *)b;

	if (p1 -> current_state < p2 -> current_state)
		return 1;
	else if (p1 -> current_state == p2 -> current_state)
	{
		if (p1 -> last_state_change < p2 -> last_state_change)
			return 1;

		return p2 -> last_state_change - p1 -> last_state_change;
	}

	return -1;
}

void sort_stats_array(struct stats *pstats, int n_stats)
{
	if (n_stats > 1)
		qsort(pstats, n_stats, sizeof(struct stats), sort_compare_func);
}

int host_is_down(struct stats *pstats, int n_stats, char *host_name)
{
	for(int loop=0; loop<n_stats; loop++)
	{
		if (pstats[loop].type == TYPE_SERVICE)
			continue;

		if (pstats[loop].current_state == 1 || // down
				pstats[loop].current_state == 2) // unreachable
		{
			if (pstats[loop].host_name != NULL && strcmp(pstats[loop].host_name, host_name) == 0)
				return loop;
		}
	}

	return -1;
}

int should_i_show_entry(struct stats *pstats, int n_stats, int cur_index, char list_all_problems, char always_notify, char also_acknowledged, char hide_ok)
{
	if (pstats[cur_index].state_type != 1)
		return 0;

	if (!list_all_problems)
	{
		if (pstats[cur_index].type != TYPE_HOST && host_is_down(pstats, n_stats, pstats[cur_index].host_name) != -1)
			return 0;

		if (pstats[cur_index].active_checks_enabled == 0 &&
				pstats[cur_index].passive_checks_enabled == 0)
			return 0;

		if (pstats[cur_index].scheduled_downtime_depth != 0)
			return 0;
	}

	if (!always_notify && pstats[cur_index].notifications_enabled == 0)
		return 0;

	if (!also_acknowledged && pstats[cur_index].problem_has_been_acknowledged == 1)
		return 0;

	if (pstats[cur_index].current_state == 0 && hide_ok)
		return 0;

	if (pstats[cur_index].current_state < 0 || pstats[cur_index].current_state > 3)
		error_exit("internal error: state %d not known", pstats[cur_index].current_state);

	return 1;
}

int find_index_by_host_and_service(struct stats *pstats, int n_stats, char *host_name, char *service_description)
{
	int loop;
	static int last = 0;

	if (service_description)
	{
		for(loop=0; loop<n_stats; loop++)
		{
			int index = (last + loop) % n_stats;
			if (pstats[index].host_name != NULL && strcmp(pstats[index].host_name, host_name) == 0 &&
					pstats[index].service_description != NULL && strcmp(pstats[index].service_description, service_description) == 0)
			{
				last = index;
				return index;
			}
		}
	}
	else
	{
		for(loop=0; loop<n_stats; loop++)
		{
			int index = (last + loop) % n_stats;
			if (pstats[index].service_description == NULL && pstats[index].host_name != NULL && strcmp(pstats[index].host_name, host_name) == 0)
			{
				last = index;
				return index;
			}
		}
	}

	return -1;
}

int check_an_age(char **message, char *descr, time_t now, time_t check_time, int max_diff, char *what)
{
	char buffer[4096] = { 0 };
	int cur_diff = (now - check_time);

	*message = 0x00;

	if (cur_diff > max_diff)
	{
		char *time_str = ctime(&check_time);
		char *dummy = strchr(time_str, '\n');
		*dummy = 0x00;

		snprintf(buffer, sizeof(buffer), "limit reached of \"%s\" for %s: %d seconds (last check: %s (%d), max diff: %d)", descr, what?what:"?", cur_diff, time_str, (int)check_time, max_diff);

		*message = strdup(buffer);

		return -1;
	}

	return 0;
}

int check_max_age_last_check(struct stats *pstats, int n_stats, int max_time_last_host_update, int max_time_oldest_host_update, int max_time_last_host_check, int max_time_oldest_host_check, int max_time_last_service_check, int max_time_oldest_service_check, int max_time_oldest_next_service_check, char **message)
{
	time_t now = time(NULL);
	int index;
	time_t most_recent_host_update = 0;
	time_t most_recent_host_check  = 0;
	time_t most_recent_last_service_check = 0;
	time_t oldest_next_service_check = now;

	for(index=0; index<n_stats; index++)
	{
		if (pstats[index].type == TYPE_HOST)
		{
			if (pstats[index].last_update != 0 && pstats[index].active_checks_enabled != 0)
			{
				most_recent_host_update = max(most_recent_host_update, pstats[index].last_update);

				if (check_an_age(message, "oldest host update", now, pstats[index].last_update, max_time_oldest_host_update, pstats[index].host_name) == -1)
					return -1;
			}

			if (pstats[index].last_check != 0 && pstats[index].active_checks_enabled != 0)
			{
				most_recent_host_check = max(most_recent_host_check, pstats[index].last_check);

				// if (check_an_age(message, "oldest host check", now, pstats[index].last_check, max_time_oldest_host_check, pstats[index].host_name) == -1)
				//	return -1;
			}
		}
		else if (pstats[index].type == TYPE_SERVICE)
		{
			if (pstats[index].last_check != 0)
			{
				char buffer[4096];

				snprintf(buffer, sizeof(buffer), "%s@%s", pstats[index].service_description, pstats[index].host_name);

				most_recent_last_service_check = max(most_recent_last_service_check, pstats[index].last_check);

				if (check_an_age(message, "oldest service check", now, pstats[index].last_check, max_time_oldest_service_check, buffer) == -1)
					return -1;
			}

			if (pstats[index].next_check != 0)
			{
				oldest_next_service_check = min(oldest_next_service_check, pstats[index].next_check);
			}
		}
	}

	if (check_an_age(message, "most recent host update", now, most_recent_host_update, max_time_last_host_update, NULL) == -1)
		return -1;

	// if (check_an_age(message, "most recent host check", now, most_recent_host_check, max_time_last_host_check, NULL) == -1)
	//	return -1;

	if (check_an_age(message, "most recent last service check", now, most_recent_last_service_check, max_time_last_service_check, NULL) == -1)
		return -1;

	if (check_an_age(message, "oldest next sevice check", now, oldest_next_service_check, max_time_oldest_next_service_check, NULL) == -1)
		return -1;

	return 0;
}

void calc_stats_stats(struct stats *pstats, int n_stats, char list_all_problems, char always_notify, char also_acknowledged, char hide_ok, int *n_critical, int *n_warning, int *n_ok, int *n_up, int *n_down, int *n_unreachable, int *n_pending)
{
	*n_critical = *n_warning = *n_ok = 0;
	*n_up = *n_down = *n_unreachable = *n_pending = 0;

	for(int loop=0; loop<n_stats; loop++)
	{
		if (pstats[loop].type == TYPE_HOST)
		{
			if (pstats[loop].current_state == 0)
				(*n_up)++;
			else if (pstats[loop].current_state == 1)
				(*n_down)++;
			else if (pstats[loop].current_state == 2)
				(*n_unreachable)++;
			else if (pstats[loop].current_state == 3)
				(*n_pending)++;
		}
		else if (pstats[loop].type == TYPE_SERVICE)
		{
			if (pstats[loop].current_state == 0)
				(*n_ok)++;
			else if (pstats[loop].current_state == 1)
				(*n_warning)++;
			else if (pstats[loop].current_state == 2)
				(*n_critical)++;
		}
		else
			error_exit("calc_stats_stats: internal error (%d)", pstats[loop].type);
	}
}

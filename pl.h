/* (C) 2006-2010 by folkert@vanheusden.com GPLv2 applies */

#define TYPE_IGNORE	0
#define TYPE_HOST	1
#define TYPE_SERVICE	2

#define V1_0_MAX_ELEMENTS	32

#define VARTYPE_PCHAR	0
#define VARTYPE_INT	1
#define VARTYPE_TIMET	2
#define VARTYPE_DOUBLE	3

#define STATS_OFFSET(x)	(int)(offsetof(struct stats, x))

#define ST_HARD			1
#define ST_SOFT			0

struct v2_0_config
{
	char *str;
	int offset;
	int type;
};

struct stats
{
	int type;

	char *host_name;
	int current_state;
	char *service_description;
	char *plugin_output;
	time_t last_state_change;
	int active_checks_enabled;
	int passive_checks_enabled;
	int notifications_enabled;
	int problem_has_been_acknowledged;
	double scheduled_downtime_depth;
	int state_type;
	int last_hard_state;
        double percent_state_change;
        double check_execution_time;
        double check_latency;
	int modified_attributes;
	int event_handler;
	int has_been_checked;
	int should_be_scheduled;
	int current_attempt;
	int max_attempts;
	int last_hard_state_change;
	int last_time_ok;
	int last_time_warning;
	int last_time_unknown;
	int last_time_critical;
	time_t last_check;
	time_t next_check;
	int check_type;
	double current_notification_number;
	int last_notification;
	int next_notification;
	int no_more_notifications;
	int event_handler_enabled;
	int acknowledgement_type;
	int flap_detection_enabled;
	int failure_prediction_enabled;
	int process_performance_data;
	int obsess;
	int obsess_over_service;
	int obsess_over_host;
	time_t last_update;
	int is_flapping;
	char * performance_data;
	char * check_command;
	int last_time_up;
	int last_time_down;
	int last_time_unreachable;
	/* newly added in 3.0 */
	char *author;
	double check_interval;
	int check_options;
	char *check_period;
	char *comment_data;
	int comment_id;
	int current_event_id;
	int current_notification_id;
	int current_problem_id;
	time_t entry_time;
	int entry_type;
	int expires;
	time_t expire_time;
	char *host_notification_period;
	int last_event_id;
	int last_problem_id;
	char *long_plugin_output;
	int next_comment_id;
	char *notification_period;
	int persistent;
	double retry_interval;
	char *service_notification_period;
	int source;
	int downtime_id;
	time_t start_time, end_time;
	int triggered_by;
	int fixed;
	int duration;
	char *comment;
};

void parse_1_0_statuslog(int fd, struct stats **pstats, int *n_stats);
void parse_2_0_statuslog(int fd, struct stats **pstats, int *n_stats);
void free_stats_array(struct stats *pstats, int n_stats);
void sort_stats_array(struct stats *pstats, int n_stats);
int host_is_down(struct stats *pstats, int n_stats, char *host_name);
int should_i_show_entry(struct stats *pstats, int n_stats, int cur_index, char list_all_problems, char always_notify, char also_acknowledged, char hide_ok);
int find_index_by_host_and_service(struct stats *pstats, int n_stats, char *host_name, char *service_description);
int check_max_age_last_check(struct stats *pstats, int n_stats, int max_time_last_host_update, int max_time_oldest_host_update, int max_time_last_host_check, int max_time_oldest_host_check, int max_time_last_service_check, int max_time_oldest_service_check, int max_time_oldest_next_service_check, char **message);
void calc_stats_stats(struct stats *pstats, int n_stats, char list_all_problems, char always_notify, char also_acknowledged, char hide_ok, int *n_critical, int *n_warning, int *n_ok, int *n_up, int *n_down, int *n_unreachable, int *n_pending);

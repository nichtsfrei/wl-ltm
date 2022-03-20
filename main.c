#include "ltm_output.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <unistd.h>
#include <wayland-client.h>

#ifndef LTM_TOGGLE_MONITOR
#define LTM_TOGGLE_MONITOR "eDP-1"
#endif
#ifndef LTM_LID_STATE_FILE_PATH
#define LTM_LID_STATE_FILE_PATH "/proc/acpi/button/lid/LID/state"
#endif
#ifndef LTM_PID_LOCATION
#define LTM_PID_LOCATION "/tmp/ltm.pid"
#endif

volatile bool run = true;

static void signal_stop_handler(int signum)
{
	(void)signum;
	run = false;
}

static void ltm_select_mode(struct ltm_display *head)
{
	if (!head->mode && head->custom_mode.refresh == 0 &&
	    head->custom_mode.width == 0 && head->custom_mode.height == 0) {
		struct ltm_display_mode *mode;
		wl_list_for_each(mode, &head->modes, link)
		{
			if (mode->preferred) {
				head->mode = mode;
				return;
			}
		}
		/* Pick first element if when there's no preferred mode */
		if (!wl_list_empty(&head->modes)) {
			head->mode =
			    wl_container_of(head->modes.next, mode, link);
		}
	}
}

static int
ltm_toggle_display(struct ltm_state *state, const char *dn, bool lidopen)
{
	struct ltm_display *current_head = NULL;
	int result = 0;

	wl_list_for_each(current_head, &state->heads, link)
	{
		if (strcmp(current_head->name, dn) == 0) {
			if (current_head->enabled != lidopen) {
				current_head->enabled = lidopen;
				if (current_head->enabled) {
					ltm_select_mode(current_head);
				}
				ltm_output_apply_state(state, false);
				result = 1;
			}
			break;
		}
	}

	return result;
}

static int ltm_lid_closed(const char *lfp)
{
	// strlen("state:      closed\n");
	const long unsigned int length_closed = 19;

	FILE *fp;
	char *line = NULL;
	size_t llen;
	ssize_t read;
	int result;

	if ((fp = fopen(lfp, "r")) == NULL)
		return -1;
	while ((read = getline(&line, &llen, fp)) != EOF)
		;
	fclose(fp);
	result = strlen(line) == length_closed ? 1 : 0;
	if (line != NULL)
		free(line);

	return result;
}

static int ltm_loop(const char *dn, const char *lfp)
{
	struct ltm_state state = {.running = true};
	struct wl_registry *registry;
	struct wl_display *display;
	int lc;
	wl_list_init(&state.heads);

	display = wl_display_connect(NULL);
	if (display == NULL) {
		syslog(LOG_ERR, "failed to connect to display\n");
		state.failed = true;
		goto exit_display;
	}
	registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, &state);
	wl_display_dispatch(display);
	wl_display_roundtrip(display);

	if (state.output_manager == NULL) {
		syslog(LOG_ERR,
		       "compositor doesn't support "
		       "wlr-output-management-unstable-v1\n");
		state.failed = true;
		goto exit;
	}

	while (state.serial == 0) {
		if (wl_display_dispatch(display) < 0) {
			syslog(LOG_ERR, "wl_display_dispatch failed\n");
			state.failed = true;
			goto exit;
		}
	}
	while (run) {

		if ((lc = ltm_lid_closed(lfp)) != -1) {

			if (ltm_toggle_display(&state, dn, !lc)) {
				syslog(LOG_DEBUG,
				       "set %s enabled to %d",
				       LTM_TOGGLE_MONITOR,
				       !lc);
				while (state.running &&
				       wl_display_dispatch(display) != -1) {
					// waiting until changes are applied
				}
				// reset to running
				state.running = true;
			}
		}
		sleep(1);
	}

exit:
	zwlr_output_manager_v1_destroy(state.output_manager);
	wl_registry_destroy(registry);
exit_display:
	wl_display_disconnect(display);
	remove(LTM_PID_LOCATION);

	return state.failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

static inline int parent(const pid_t pid)
{
	FILE *fp = NULL;
	printf("started (%d) to watch for %s to handle %s\n",
	       pid,
	       LTM_LID_STATE_FILE_PATH,
	       LTM_TOGGLE_MONITOR);
	if ((fp = fopen(LTM_PID_LOCATION, "w")) == NULL) {
		fprintf(stderr,
			"failed to write pid file (%s)\n",
			LTM_PID_LOCATION);
		return EXIT_FAILURE;
	}
	fprintf(fp, "%d\n", pid);
	return EXIT_SUCCESS;
}

int main(void)
{

	pid_t pid, sid;

	pid = fork();
	if (pid < 0) {
		return EXIT_FAILURE;
	}
	if (pid > 0) {
		return parent(pid);
	}

	umask(0);

	sid = setsid();
	if (sid < 0) {
		return EXIT_FAILURE;
	}

	if ((chdir("/")) < 0) {
		return EXIT_FAILURE;
	}

	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	signal(SIGINT, signal_stop_handler);
	signal(SIGABRT, signal_stop_handler);
	signal(SIGTERM, signal_stop_handler);
	return ltm_loop(LTM_TOGGLE_MONITOR, LTM_LID_STATE_FILE_PATH);
}

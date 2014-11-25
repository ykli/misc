#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

int parse_key_value_config(int * const argc, char ***argv, char *filename)
{
	FILE *fp = NULL;
	char buf[1024];
	char *ubuf = NULL;
	char *commit_pos = NULL;
	int commit_flag = '#';
	int kv_flag = '=';
	char *value_pos = NULL;
	int count = 0;
	char **kvargv = NULL;
	char tmp_kvargv[1024];
	int i = 0;

	if ((fp = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "fopen %s failed:%s\n", filename, strerror(errno));
		goto err_fopen_parse_file;
	}

	if ((kvargv = (char **)malloc(sizeof(char *) * 1024)) == NULL) {
		goto err_malloc_kvargv;
	}
	if ((kvargv[count++] = strdup("argv")) == NULL) {
		goto err_strdup_argv0;
	}
	memset(buf, 0, sizeof(buf));
	while(fgets(buf, sizeof(buf), fp) != NULL) {
		if ((buf[0] != '\0') && ((commit_pos = strchr(buf, commit_flag)) != NULL)) {
			*commit_pos = '\0';
		}
		if ((buf[0] != '\0') && ((commit_pos = strrchr(buf, commit_flag)) != NULL)) {
			*commit_pos = '\0';
		}
		if ((ubuf = strtok(buf, "\n#\t ")) != NULL) {
			if ((value_pos = strchr(ubuf, kv_flag)) != NULL) {
				*value_pos++ = '\0';
			}
			if (strlen(ubuf) > 1) {
				tmp_kvargv[0] = '-';
				tmp_kvargv[1] = '-';
				tmp_kvargv[2] = '\0';
				*(value_pos - 1) = '=';
				if (!(kvargv[count++] = strdup(strcat(tmp_kvargv, ubuf)))) {
					count--;
					goto err_strdup;
				}
			} else {
				tmp_kvargv[0] = '-';
				tmp_kvargv[1] = '\0';
				if (!(kvargv[count++] = strdup(strcat(tmp_kvargv, ubuf)))) {
					count--;
					goto err_strdup;
				}
				if (value_pos) {
					if (!(kvargv[count++] = strdup(value_pos))) {
						count--;
						goto err_strdup;
					}
				}
			}
			while((ubuf = strtok(NULL, "\n#\t ")) != NULL) {
				if ((value_pos = strchr(ubuf, kv_flag)) != NULL) {
					*value_pos++ = '\0';
				}
				if (strlen(ubuf) > 1) {
					tmp_kvargv[0] = '-';
					tmp_kvargv[1] = '-';
					tmp_kvargv[2] = '\0';
					*(value_pos - 1) = '=';
					if (!(kvargv[count++] = strdup(strcat(tmp_kvargv, ubuf)))) {
						count--;
						goto err_strdup;
					}
				} else {
					tmp_kvargv[0] = '-';
					tmp_kvargv[1] = '\0';
					if (!(kvargv[count++] = strdup(strcat(tmp_kvargv, ubuf)))) {
						count--;
						goto err_strdup;
					}
					if (value_pos) {
						if (!(kvargv[count++] = strdup(value_pos))) {
							count--;
							goto err_strdup;
						}
					}
				}
			}
		}
	}
err_strdup:
	if (count > 1) {
		if ((*argv = malloc(sizeof(char *) * (count + 1))) == NULL) {
			goto err_malloc_argv;
		} else {
			memcpy(*argv, kvargv, sizeof(char *) * count);
			(*argv)[count] = NULL;
			*argc = count;
			free(kvargv);
		}
	} else {
		goto err_strdup_argv0;
	}

	fclose(fp);

	return 0;

err_malloc_argv:
	for (i = 0; i < count; i++) {
		free(kvargv[i]);
	}
err_strdup_argv0:
	*argc = 0;
	argv = NULL;
	free(kvargv);
err_malloc_kvargv:
	fclose(fp);
err_fopen_parse_file:
	return -1;
}

void destroy_argc_argv(int argc, char **argv)
{
	int i = 0;
	for (i = 0; i < argc + 1; i++) {
		free(argv[i]);
	}
	free(argv);
}

void dump_argc_argv(int argc, char **argv)
{
	int i = 0;
	for (i = 0; i < argc + 1; i++) {
		printf("argv[%d] = %s\n", i, argv[i]);
	}
}

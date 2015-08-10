/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 1999-2010 Carnegie Mellon University.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced
 * Research Projects Agency and the National Science Foundation of the
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */
/*
 * continuous.c - Simple pocketsphinx command-line application to test
 *                both continuous listening/silence filtering from microphone
 *                and continuous file transcription.
 */

/*
 * This is a simple example of pocketsphinx application that uses continuous listening
 * with silence filtering to automatically segment a continuous stream of audio input
 * into utterances that are then decoded.
 *
 * Remarks:
 *   - Each utterance is ended when a silence segment of at least 1 sec is recognized.
 *   - Single-threaded implementation for portability.
 *   - Uses audio library; can be replaced with an equivalent custom library.
 */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <linux/kd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sphinxbase/err.h>
#include <sphinxbase/ad.h>
#include <pocketsphinx/pocketsphinx.h>

static const arg_t cont_args_def[] =
{
		POCKETSPHINX_OPTIONS,
		/* Argument file. */
		{
				"-argfile",
				ARG_STRING,
				NULL,
				"Argument file giving extra arguments."
		},
		{
				"-adcdev",
				ARG_STRING,
				NULL,
				"Name of audio device to use for input."
		},
	    {
	    		"-inmic",
				ARG_BOOLEAN,
				"yes",
				"Transcribe audio from microphone."
	    },
		{
				"-time",
				ARG_BOOLEAN,
				"no",
				"Print word times in file transcription."
		},
		CMDLN_EMPTY_OPTION
};

static ps_decoder_t *ps;
static cmd_ln_t *config;

static void led_on(int fd)
{
	int status;

	if ((ioctl(fd, KDSETLED, LED_CAP)) == -1)
	{
		perror("ioctl");
	    E_FATAL("Could not open console device.");
	}
}

static void led_off(int fd)
{
	int status;

	if ((ioctl(fd, KDSETLED, 0x00)) == -1)
	{
		perror("ioctl");
	    E_FATAL("Could not open console device.");
	}
}

static void call_cmd(const char *v_cmd)
{
	if (*v_cmd != 0)
	{
		printf("Voice command: %s.\n", v_cmd);
	}
}

/* Sleep for specified msec */
static void sleep_msec(int32 ms)
{
	struct timeval tmo;

	tmo.tv_sec = 0;
	tmo.tv_usec = ms * 1000;

	select(0, NULL, NULL, NULL, &tmo);
}

/*
 * Main utterance processing loop:
 *     for (;;) {
 *        start utterance and wait for speech to process
 *        decoding till end-of-utterance silence will be detected
 *        print utterance result;
 *     }
 */
static void recognize_from_microphone()
{
	ad_rec_t *ad;
	int16 adbuf[2048];
	uint8 utt_started, in_speech;
	int32 k;
	int32 score;
	char const *hyp;
	int fd;

    /* Keyboard LED ioctl(). */
	if ((fd = open("/dev/console", O_NOCTTY)) == -1)
	{
		perror("open");
		E_FATAL("Could not open console device.");
	}

	if ((ad = ad_open_dev(cmd_ln_str_r(config, "-adcdev"),
			(int) cmd_ln_float32_r(config, "-samprate"))) == NULL)
	{
		E_FATAL("Failed to open audio device\n");
	}

	if (ad_start_rec(ad) < 0)
	{
		E_FATAL("Failed to start recording\n");
	}

	if (ps_start_utt(ps) < 0)
	{
		E_FATAL("Failed to start utterance\n");
	}

	utt_started = FALSE;
	printf("READY....\n");

	for (;;)
	{
		if ((k = ad_read(ad, adbuf, 2048)) < 0)
		{
			E_FATAL("Failed to read audio\n");
		}
		ps_process_raw(ps, adbuf, k, FALSE, FALSE);
		in_speech = ps_get_in_speech(ps);
		if (in_speech && !utt_started)
		{
			utt_started = TRUE;
			printf("Listening...\n");
			led_on(fd);
		}
		if (!in_speech && utt_started)
		{
			/* speech -> silence transition, time to start new utterance  */
			ps_end_utt(ps);
			hyp = ps_get_hyp(ps, NULL);
			score = ps_get_prob(ps);
			if (hyp != NULL)
			{
				call_cmd(hyp);
				printf("%s, score %d\n", hyp, score);
			}

			if (ps_start_utt(ps) < 0)
			{
				E_FATAL("Failed to start utterance\n");
			}
			utt_started = FALSE;
			printf("READY....\n");
			led_off(fd);
		}
		sleep_msec(100);
	}
	ad_close(ad);
    close(fd);
}

int main(int argc, char *argv[])
{
	char const *cfg;

	/* Disable logging */
	err_set_logfile("/dev/null");

	config = cmd_ln_parse_r(NULL, cont_args_def, argc, argv, TRUE);

	/* Handle argument file as -argfile. */
	if (config && (cfg = cmd_ln_str_r(config, "-argfile")) != NULL)
	{
		config = cmd_ln_parse_file_r(config, cont_args_def, cfg, FALSE);
	}

	ps_default_search_args(config);
	ps = ps_init(config);
	if (ps == NULL)
	{
		cmd_ln_free_r(config);
		return 1;
	}

	recognize_from_microphone();
	ps_free(ps);
	cmd_ln_free_r(config);

	return 0;
}


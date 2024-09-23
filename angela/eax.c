/*
    ENHANCED ANGELA (EAX) (ANGELA-2)

	Enhanced Angela Encryption Implementation

    Copyright (C) 2008-2013 RedStar Technologies, All Rights Reserved.
    Written by J. Palencia (ciachn@gmail.com)
*/

	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>

	#define BYTES 32
	#define BYTES_MASK 31

	typedef struct {
		char in[BYTES];
		char out[BYTES];
		int p[BYTES];
		int q[BYTES];
		char *k;
		int ilevel, kl, ki;
	}
	eax_context;

	int primes[] = {
		67, 53, 419, 709, 373, 101, 401, 761, 997, 739, 641, 313, 61, 83, 59, 769,
		1129, 2969, 4937, 5743, 7237, 6571, 8167, 8713, 8933, 5179, 3673, 3727, 3083, 4817, 4523, 4507
	};

	int rol (int v, int n, int m)
	{
		v &= (1 << m) - 1;
		return ((v << n) | ((v >> (m-n)) & ((1 << n) - 1))) & ((1 << m) - 1);
	}

	int ror (int v, int n, int m)
	{
		return rol (v, m-n, m);
	}

	eax_context *eax_init (char *k)
	{
		int i, j, sumL, sumH;

		eax_context *ctx = (eax_context *)malloc(sizeof(eax_context));
		memset(ctx, 0, sizeof(eax_context));

		for (i = 0; i < BYTES; i++) {
			ctx->p[i] = primes[i];
			ctx->q[i] = primes[BYTES-i-1];
		}

		ctx->k = k;
		ctx->kl = strlen(k);
		ctx->ki = 0;

		if (ctx->kl != 0)
		{
            for (i = j = 0; i < ctx->kl; i++) j += 3*ctx->k[i];
		}
		else
			j = 0xA527;

		sumH = (j >> 8) & 0xFF;
		sumL = j & 0xFF;

		for (i = 0; i < BYTES; i++)
		{
			ctx->p[i&BYTES_MASK] = ror (ctx->p[i] + sumH, sumH & 5, 16) - ctx->p[(i+1)&BYTES_MASK];
			ctx->q[i&BYTES_MASK] = rol (ctx->q[i] + ctx->q[(i+1)&BYTES_MASK], sumL & 3, 16) - sumL;

			j = ctx->p[i] & ctx->q[i];

			sumH += (j & 0xF0);
			sumL -= (j & 0x0F);
		}

		return ctx;
	}

	int eax_input_full (eax_context *ctx)
	{
		return ctx->ilevel == BYTES;
	}

	int eax_input_space (eax_context *ctx)
	{
		return BYTES - ctx->ilevel;
	}

	int eax_input_empty (eax_context *ctx)
	{
		return ctx->ilevel == 0;
	}

	int eax_real_size (int size)
	{
		return (size + BYTES - 1) & -BYTES;
	}

	int eax_bottom_bytes (int size)
	{
		return (size & BYTES_MASK) ? (size & BYTES_MASK) : BYTES;
	}

	int eax_encrypt_block (eax_context *ctx)
	{
		int i, k, k2;

		if (ctx->kl != 0)
		{
			for (i = 0; i < BYTES; i++)
			{
				ctx->q[i] = rol (ctx->q[i] + ctx->k[ctx->ki], ctx->k[ctx->ki] & 3, 16);
				ctx->ki = ++ctx->ki % ctx->kl;
			}
		}

		for (i = 0; i < BYTES; i++)
			ctx->q[i] = ror (ctx->q[i], ctx->q[i] & 5, 16) ^ rol (ctx->p[i], ctx->p[i] & 7, 16);

		for (i = 0; i < BYTES; i++)
		{
			k = ctx->in[i];

			k2 = rol(k, 4, 8) ^ ctx->p[i];
			k2 = k2 + ror(ctx->q[BYTES-i-1], ctx->p[i] & 3, 8);

			k &= 0xFF; k2 &= 0xFF;
			ctx->p[i] ^= (k + k2);
			ctx->out[i] = k2;
		}

		ctx->ilevel = 0;
		return 1;
	}

	int eax_decrypt_block (eax_context *ctx)
	{
		int i, k, k2;

		if (ctx->kl != 0)
		{
			for (i = 0; i < BYTES; i++)
			{
				ctx->q[i] = rol (ctx->q[i] + ctx->k[ctx->ki], ctx->k[ctx->ki] & 3, 16);
				ctx->ki = ++ctx->ki % ctx->kl;
			}
		}

		for (i = 0; i < BYTES; i++)
			ctx->q[i] = ror (ctx->q[i], ctx->q[i] & 5, 16) ^ rol (ctx->p[i], ctx->p[i] & 7, 16);

		for (i = 0; i < BYTES; i++)
		{
			k2 = ctx->in[i];

			k = k2 - ror(ctx->q[BYTES-i-1], ctx->p[i] & 3, 8);
			k = ror(k ^ ctx->p[i], 4, 8);

			k &= 0xFF; k2 &= 0xFF;
			ctx->p[i] ^= (k + k2);
			ctx->out[i] = k;
		}

		ctx->ilevel = 0;
		return 1;
	}

	int eax_feed_byte (eax_context *ctx, int byte)
	{
		if (ctx->ilevel == BYTES) return 0;

		ctx->in[ctx->ilevel++] = (byte + 0x100) & 0xFF;
		return 1;
	}

	int main(int argc, char* argv[])
	{
		FILE *in, *out; int ch;
		eax_context *ctx;
		int file_size, bytes = 0;
		int flush = 0;
        int salt = rand() & 0xFF;
        int o = 0;
		int first = 1;

		if (argc < 5) {
			printf ("Usage: eax <e|d> key input output");
			return 0;
		}

		ctx = eax_init (argv[2]);

		in = fopen (argv[3], "rb");
		if (!in) return 0;

		out = fopen (argv[4], "wb");
		if (!out) return 0;

		fseek (in, 0, SEEK_END);
		file_size = ftell (in);
		rewind (in);

        if (argv[1][0] == 'e')
		{
            eax_feed_byte (ctx, salt);
		}

		while ((ch = getc(in)) != EOF)
		{
			if (eax_input_full (ctx))
			{
				if (flush)
				{
					if (first && argv[1][0] == 'd')
					{
						fwrite (&ctx->out[1], 1, BYTES-1, out);
						first = 0;
						salt = ctx->out[0];
					}
					else
					{
						fwrite (&ctx->out, 1, BYTES, out);
					}
				}

				if (argv[1][0] == 'd') {
					eax_decrypt_block (ctx);
				} else {
					eax_encrypt_block (ctx);
				}

				flush = 1;
			}

			eax_feed_byte (ctx, ch);
			bytes++;

			if (!(bytes & 4095)) printf ("\r %.2f%% Completed...", (bytes*100.0/file_size));
		}

		printf ("\r %.2f%% Completed...", (bytes*100.0/file_size));

		if (flush) {
			fwrite (&ctx->out, 1, BYTES, out);
			flush = 0;
		}

		if (!eax_input_empty (ctx) && argv[1][0] == 'd')
		{
			eax_decrypt_block (ctx);
			flush = 1;
		}

		if (flush)
		{
			if (argv[1][0] == 'd')
			{
				o = ctx->out[BYTES-1];

				if (ctx->out[BYTES-o-2] != salt)
				{
					printf ("Simple security check failed. Unable to decrypt.\n");
					fclose (out);
					unlink (argv[4]);
					return -1;
				}

				fwrite (&ctx->out, 1, BYTES-o-2, out);
			}
			else
				fwrite (&ctx->out, 1, BYTES, out);
		}

		if (argv[1][0] == 'e')
		{
            eax_feed_byte (ctx, salt);

            while (eax_input_space (ctx) != 1)
			{
				eax_feed_byte (ctx, rand());
				o++;
			}

            eax_feed_byte (ctx, o);

			eax_encrypt_block (ctx);
			fwrite (&ctx->out, 1, BYTES, out);
		}

		fclose (in);
		fclose (out);

		return 0;
	}

/*	$OpenBSD: blf.h,v 1.3 2001/05/15 02:40:35 deraadt Exp $ */

/*
 * Blowfish - a fast block cipher designed by Bruce Schneier
 *
 * Copyright 1997 Niels Provos <provos@physnet.uni-hamburg.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *	  must display the following acknowledgement:
 *		This product includes software developed by Niels Provos.
 * 4. The name of the author may not be used to endorse or promote products
 *	  derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _BLF_H_
#define _BLF_H_

/* Schneier states the maximum key length to be 56 bytes.
 * The way how the subkeys are initalized by the key up
 * to (N+2)*4 i.e. 72 bytes are utilized.
 * Warning: For normal blowfish encryption only 56 bytes
 * of the key affect all cipherbits.
 */

#define BLF_N	16				/* Number of Subkeys */
#define BLF_MAXKEYLEN ((BLF_N-2)*4)		/* 448 bits */

/* Blowfish context */
typedef struct BlowfishContext
{
	uint32		S[4][256];		/* S-Boxes */
	uint32		P[BLF_N + 2];	/* Subkeys */
}			blf_ctx;

/* Raw access to customized Blowfish
 *	blf_key is just:
 *	Blowfish_initstate( state )
 *	Blowfish_expand0state( state, key, keylen )
 */

void		Blowfish_encipher(blf_ctx *, uint32 *);
void		Blowfish_decipher(blf_ctx *, uint32 *);
void		Blowfish_initstate(blf_ctx *);
void		Blowfish_expand0state(blf_ctx *, const uint8 *, uint16);
void		Blowfish_expandstate
			(blf_ctx *, const uint8 *, uint16, const uint8 *, uint16);

/* Standard Blowfish */

void		blf_key(blf_ctx *, const uint8 *, uint16);
void		blf_enc(blf_ctx *, uint32 *, uint16);
void		blf_dec(blf_ctx *, uint32 *, uint16);

/* Converts uint8 to uint32 */
uint32		Blowfish_stream2word(const uint8 *, uint16, uint16 *);

void		blf_ecb_encrypt(blf_ctx *, uint8 *, uint32);
void		blf_ecb_decrypt(blf_ctx *, uint8 *, uint32);

void		blf_cbc_encrypt(blf_ctx *, uint8 *, uint8 *, uint32);
void		blf_cbc_decrypt(blf_ctx *, uint8 *, uint8 *, uint32);

#endif

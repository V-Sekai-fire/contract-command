// The three shapes a reply may take, written and then read by a real virtual machine.
//
// RFD 0124 carries a reply as an Elixir term in CBOR: an atom is tag 39, a tuple is an array,
// a map is a map, a binary is a text string. This writes one file per shape. `proof/term.exs`
// decodes them through `elixir/reply.ex` and matches them the way a caller does, which is the
// only check that establishes the claim -- a C program agreeing with a C program says nothing
// about Elixir.
//
// SPDX-License-Identifier: Apache-2.0

#include "weft/cbor.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

static void emit(const char *dir, const char *name, const unsigned char *buf, size_t n,
		int over) {
	char path[512];
	snprintf(path, sizeof(path), "%s/%s", dir, name);
	FILE *f = fopen(path, "wb");
	if (!f) {
		fprintf(stderr, "FAIL cannot write %s\n", path);
		++failures;
		return;
	}
	fwrite(buf, 1, n, f);
	fclose(f);
	// A truncated reply decodes as a short one and the reader cannot tell, so `over` is the
	// only honest report of a buffer that ran out.
	if (over) {
		fprintf(stderr, "FAIL %s overflowed its buffer\n", name);
		++failures;
	} else {
		printf("ok   wrote %s, %zu bytes\n", name, n);
	}
}

int main(int argc, char **argv) {
	const char *dir = argc > 1 ? argv[1] : ".";
	unsigned char buf[512];

	{ // {:error, :no_engine}
		weft_cbor_t c = weft_cbor_to(buf, sizeof(buf));
		weft_cbor_error(&c, "no_engine");
		emit(dir, "bare.cbor", buf, c.n, weft_cbor_over(&c));
	}

	{ // {:error, {:res_below_minimum, %{got: 512, minimum: 1280}}}
		weft_cbor_t c = weft_cbor_to(buf, sizeof(buf));
		weft_cbor_error_detail(&c, "res_below_minimum", 2);
		weft_cbor_atom(&c, "got");
		weft_cbor_int(&c, 512);
		weft_cbor_atom(&c, "minimum");
		weft_cbor_int(&c, 1280);
		emit(dir, "detail.cbor", buf, c.n, weft_cbor_over(&c));
	}

	{ // {:ok, %{layers: 9, ms: 171260, sidecar: "test.psd"}}
		weft_cbor_t c = weft_cbor_to(buf, sizeof(buf));
		weft_cbor_ok_map(&c, 3);
		weft_cbor_atom(&c, "layers");
		weft_cbor_int(&c, 9);
		weft_cbor_atom(&c, "ms");
		weft_cbor_int(&c, 171260);
		weft_cbor_atom(&c, "sidecar");
		weft_cbor_text(&c, "test.psd");
		emit(dir, "ok.cbor", buf, c.n, weft_cbor_over(&c));
	}

	{ // A buffer too small must say so rather than write a short reply that decodes.
		unsigned char tiny[4];
		weft_cbor_t c = weft_cbor_to(tiny, sizeof(tiny));
		weft_cbor_error_detail(&c, "res_below_minimum", 2);
		if (weft_cbor_over(&c)) {
			printf("ok   a reply that does not fit reports over, rather than truncating\n");
		} else {
			printf("FAIL a reply that did not fit reported no overflow\n");
			++failures;
		}
	}

	printf("%s\n", failures ? "term: FAILED" : "term: all checks passed");
	return failures ? 1 : 0;
}

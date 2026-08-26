#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "efex-common.h"
#include "libefex.h"

int main(int argc, char **argv) {
	struct sunxi_efex_ctx_t ctx = {0};
	struct sunxi_fes_verify_resp_t response = {0};
	int ret;

	if (argc != 3 && argc != 4) {
		fprintf(stderr, "usage: %s START_SECTOR SIZE_BYTES [OUTPUT]\n", argv[0]);
		return 2;
	}

	uint32_t start = (uint32_t)strtoul(argv[1], NULL, 0);
	uint64_t size = strtoull(argv[2], NULL, 0);

	ret = sunxi_scan_usb_device(&ctx);
	if (ret == EFEX_ERR_SUCCESS)
		ret = sunxi_usb_init(&ctx);
	if (ret == EFEX_ERR_SUCCESS)
		ret = sunxi_efex_init(&ctx);
	if (ret != EFEX_ERR_SUCCESS) {
		fprintf(stderr, "open failed: %s\n", sunxi_efex_strerror(ret));
		return ret;
	}

	if (argc == 4) {
		char *data = malloc(size);
		FILE *output;
		if (!data) {
			sunxi_usb_exit(&ctx);
			return 3;
		}
		ret = sunxi_efex_fes_spinand_up(&ctx, data, size, start << 9,
				(enum sunxi_fes_data_type_t)0x8040);
		output = ret == EFEX_ERR_SUCCESS ? fopen(argv[3], "wb") : NULL;
		if (!output || fwrite(data, 1, size, output) != size)
			ret = ret == EFEX_ERR_SUCCESS ? 4 : ret;
		if (output)
			fclose(output);
		free(data);
	} else {
		ret = sunxi_efex_fes_verify_value(&ctx, start, size, &response);
	}
	if (ret == EFEX_ERR_SUCCESS && argc == 3)
		printf("start=%" PRIu32 " size=%" PRIu64
		       " fes=0x%08x media=0x%08x flag=0x%08x\n",
		       start, size, (uint32_t)response.fes_crc,
		       (uint32_t)response.media_crc, response.flag);
	else if (ret != EFEX_ERR_SUCCESS)
		fprintf(stderr, "verify failed: %s\n", sunxi_efex_strerror(ret));

	sunxi_usb_exit(&ctx);
	return ret;
}

#ifndef __PADDING_H__
#define __PADDING_H__

template <typename PixelType>
void PadCorner(PixelType *p, PixelType v, int32_t hPad, int32_t vPad, int32_t refPitch) {
	for (int32_t i = 0; i < vPad; i++) {
		for (int32_t j = 0; j < hPad; j++)
			p[j] = v;
		p += refPitch;
	}
}

template <typename PixelType>
void PadReferenceFrame(uint8_t *refFrame8, int32_t refPitch, int32_t hPad, int32_t vPad, int32_t width, int32_t height) {
	refPitch /= sizeof(PixelType);
	PixelType *refFrame = (PixelType *)refFrame8;
	PixelType value;
	PixelType *pfoff = refFrame + vPad * refPitch + hPad;
	PixelType *p;
	PadCorner<PixelType>(refFrame, pfoff[0], hPad, vPad, refPitch);
	PadCorner<PixelType>(refFrame + hPad + width, pfoff[width - 1], hPad, vPad, refPitch);
	PadCorner<PixelType>(refFrame + (vPad + height) * refPitch,
		pfoff[(height - 1) * refPitch], hPad, vPad, refPitch);
	PadCorner<PixelType>(refFrame + hPad + width + (vPad + height) * refPitch,
		pfoff[(height - 1) * refPitch + width - 1], hPad, vPad, refPitch);
	for (int32_t i = 0; i < width; i++) {
		value = pfoff[i];
		p = refFrame + hPad + i;
		for (int32_t j = 0; j < vPad; j++) {
			p[0] = value;
			p += refPitch;
		}
	}
	for (int32_t i = 0; i < height; i++) {
		value = pfoff[i*refPitch];
		p = refFrame + (vPad + i) * refPitch;
		for (int32_t j = 0; j < hPad; j++)
			p[j] = value;
	}
	for (int32_t i = 0; i < height; i++) {
		value = pfoff[i * refPitch + width - 1];
		p = refFrame + (vPad + i) * refPitch + width + hPad;
		for (int32_t j = 0; j < hPad; j++)
			p[j] = value;
	}
	for (int32_t i = 0; i < width; i++) {
		value = pfoff[i + (height - 1) * refPitch];
		p = refFrame + hPad + i + (height + vPad) * refPitch;
		for (int32_t j = 0; j < vPad; j++) {
			p[0] = value;
			p += refPitch;
		}
	}
}

#endif

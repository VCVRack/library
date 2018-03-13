struct SevenSegmentDisplay : Widget {
	SevenSegmentDisplay(Vec position, float size, char *display);

	void draw(NVGcontext *vg) override;

	Vec position;
	float size;
	char *display;
};

struct SevenSegmentDot : Widget {
	SevenSegmentDot(Vec position, float size);

	void draw(NVGcontext *vg) override;

	Vec position;
	float size;
};

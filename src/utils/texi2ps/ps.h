/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#define PSF_bold	0x01
#define PSF_italic	0x02

#define PSF_times	0x00
#define PSF_courier	0x04
#define PSF_symbol	0x08
#define PSF_helvetica	0x0c
#define PSF_dingbats	0x10

extern float *font_metrics;

extern int ps_font;
extern int ps_fontsize;

void psprintf(char *fmt, ...);
void psputw(char *word);
void pscomment(char *fmt, ...);

void psf_pushfont(int flags);
void psf_pushset(int flags);
void psf_pushreset(int flags);
void psf_pushscale(int size);
void psf_pop(void);
void psf_setfont(void);

void psdone(void);

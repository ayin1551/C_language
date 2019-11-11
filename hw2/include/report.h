
/*
 * Type definitions for report functions.
 */

void histo(FILE *fd,int bins[50],float min,float max,int cnt);
void reportparams(FILE *fd,char *fn,Course *c);
void reportfreqs(FILE *fd,Stats *s);
float interpolatequantile(Freqs *fp,int n,float q);
void reportquantilesummaries(FILE *fd,Stats *s);
void reportquantiles(FILE *fd,Stats *s);
void reportmoments(FILE *fd,Stats *s);
void reportscores(FILE *fd,Course *c,int nm);
void reportcomposites(FILE *fd,Course *c,int nm);
void reporthistos(FILE *fd,Course *c,Stats *s);
void reporttabs(FILE *fd, Course *c);

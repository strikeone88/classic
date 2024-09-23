#ifdef Pretty

%surface 7 8 "output/" $FILE black (bmp);

inline-bmp ($FILE);
filter (value: soften-lo);

#else

%surface 7 8 "output/" $FILE black (bmp);

inline-bmp ($FILE);

#endif

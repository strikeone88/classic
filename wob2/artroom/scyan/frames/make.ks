#ifdef Pretty

%surface 16 21 "temp" black (bmp);

inline-bmp ($FILE);
filter (value: shatter);
filter (value: soften-med);

inline-bmp ($FILE, blend: 100%);
filter (value: shatter);
filter (value: shatter);
filter (value: soften-med);

inline-bmp ($FILE, blend: 100%);
filter (value: shatter);
filter (value: shatter);
filter (value: soften-med);

%surface 16 21 "output/" $FILE black (bmp);

inline-bmp ("temp.bmp");
inline-bmp ($FILE, blend: 100%);

#else

%surface 16 21 "output/" $FILE black (bmp);
inline-bmp ($FILE);

#endif

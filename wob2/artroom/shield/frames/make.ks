%surface 80 80 "temp" black (bmp);

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

inline-bmp ($FILE, blend: 100%);
filter (value: shatter);
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

%surface 80 80 "output/" $FILE black (bmp);

inline-bmp ("temp.bmp");
inline-bmp ($FILE, blend: 100%);

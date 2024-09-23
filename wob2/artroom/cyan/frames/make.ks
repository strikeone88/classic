%surface 32 41 "temp1" black (bmp);

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

%surface 32 41 "output/" $FILE black (bmp);

inline-bmp ("temp1.bmp");
inline-bmp ($FILE, blend: 100%);

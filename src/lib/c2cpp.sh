find ft8_lib -type f -name "*.c" | 
    sed "s/\.c$//" | 
    xargs -I% mv -iv %.c %.cpp

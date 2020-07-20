# About
This sketch allows your Serpente CircuitPython board's SPI connected flash to be read and written to by flashrom using the serprog serial flashing protocol.

# Usage
Flashrom should be able to autodetect the flash chip, if it doesn't then something is really wrong.
If that happens please let me know.

Reading flash contents: 
``` 
flashrom --programmer serprog:dev=/dev/ttyS8:115200 --read flash.bin
```

Erasing flash: 
``` 
flashrom --programmer serprog:dev=/dev/ttyS8:115200 --erase
```

Writing flash:  
```
flashrom --programmer serprog:dev=/dev/ttyS8:115200 --write flash.bin 
```

Substitute /dev/ttyS8 for whichever port your Serpente shows up as.
You can leave the baud rate at 115200 since it's connected via USB CDC it doesn't matter.

# LEDs
Green: Ready  
Yellow: Working  

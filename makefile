objects=main.o
source=main.c
cc=gcc
output=getbook

$(output): $(objects)
	cc  -D_GNU_SOURCE -o $(output) $(objects) -g -lcurl -lpthread `mysql_config --cflags --libs` -liniparser -levent

$(objects):
	cc  -D_GNU_SOURCE -c $(source) -g `mysql_config --cflags --libs` -liniparser -levent


.PHONY:clean
clean:
	-rm $(output) $(objects)

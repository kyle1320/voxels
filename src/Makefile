CC = gcc
CFLAGS = -Wall -std=c99 -O3 -I '/usr/local/include/'
LIBFLAGS = -L/usr/local/lib -lglfw3 -lglew -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -lpthread

main: main.o voxels.o loadShaders.o matrix.o loadTexture.o mesh.o physics.o model.o color.o light.o logic.o
	$(CC) $(CFLAGS) $(LIBFLAGS) -o main $^

clean:
	rm *.o

main.o:        main.c main.h voxels.h matrix.h mesh.h physics.h color.h
voxels.o:      voxels.c voxels.h matrix.h mesh.h color.h
loadShaders.o: loadShaders.c
loadTexture.o: loadTexture.c
matrix.o:      matrix.c matrix.h
mesh.o:        mesh.c mesh.h main.h matrix.h color.h
physics.o:     physics.c physics.h matrix.h
model.o:       model.c model.h voxels.h color.h
color.o:       color.c color.h
light.o:       light.c light.h mesh.h
logic.o:       logic.c logic.h

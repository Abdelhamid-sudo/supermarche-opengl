CXX      = g++
CXXFLAGS = -std=c++11 -O2 -Wall
LDLIBS   = -lglfw -lGLEW -lGL

COMMON   = Camera.cpp Mesh.cpp ShaderProgram.cpp Texture2D.cpp
HEADERS  = Camera.h Mesh.h ShaderProgram.h Texture2D.h

DEMOS    = Lighting_Dir Lighting_Point Lighting_Spot Lighting_Phong Lighting_Materials Lighting_Multiple

.PHONY: all demos run run-neon run-port record clean

all: supermarche neon_noir scene_virtuelle

supermarche: Supermarche.cpp $(COMMON) $(HEADERS)
	$(CXX) $(CXXFLAGS) Supermarche.cpp $(COMMON) -o $@ $(LDLIBS)

neon_noir: NeonNoir.cpp CameraPath.h $(COMMON) $(HEADERS)
	$(CXX) $(CXXFLAGS) NeonNoir.cpp $(COMMON) -o $@ $(LDLIBS)

scene_virtuelle: Scene_Virtuelle.cpp $(COMMON) $(HEADERS)
	$(CXX) $(CXXFLAGS) Scene_Virtuelle.cpp $(COMMON) -o $@ $(LDLIBS)

demos: $(DEMOS)

$(DEMOS): %: %.cpp $(COMMON) $(HEADERS)
	$(CXX) $(CXXFLAGS) $< $(COMMON) -o $@ $(LDLIBS)

run: supermarche
	./supermarche

run-neon: neon_noir
	./neon_noir

run-port: scene_virtuelle
	./scene_virtuelle

record: neon_noir
	./neon_noir --record

clean:
	rm -f supermarche neon_noir scene_virtuelle $(DEMOS) shot_*.ppm

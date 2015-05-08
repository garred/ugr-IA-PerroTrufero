CPP  = g++ -O3
CC   = gcc
OBJ  = main.o agent.o environment.o GUI.o evaluator.o random_num_gen.o  include/glui.o include/glui_bitmap_img_data.o include/glui_bitmaps.o include/glui_button.o include/glui_column.o include/glui_control.o include/glui_edittext.o include/glui_node.o include/glui_panel.o include/glui_radio.o include/glui_rollout.o include/glui_spinner.o include/glui_statictext.o include/glui_string.o include/glui_window.o include/glui_filebrowser.o include/glui_list.o include/glui_scrollbar.o include/glui_tree.o
LIBS =  -lGL -lGLU -lglut
BIN  = agent
RM = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after


$(BIN): $(OBJ)
	$(CPP) $(OBJ) -o $(BIN) $(LIBS) -fopenmp #

agent.o: agent.cpp agent.h
	$(CPP) -c agent.cpp -o agent.o $(CXXFLAGS)


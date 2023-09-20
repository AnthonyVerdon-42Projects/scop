SRCS := srcs/main.cpp \
		includes/glad/glad.cpp \
		includes/stb_image/stb_image.cpp \
		srcs/setupGLFW.cpp \
		srcs/input.cpp \
		srcs/classes/Shader.cpp

OBJS := $(SRCS:.cpp=.o)

NAME := scop

COMPILER ?= c++

RM		:= rm -f

CFLAGS 	:= -Wall -Werror -Wextra -g

LIBRARIES := -Lincludes/glfw3/build/src -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl

.cpp.o:
			${COMPILER} ${CFLAGS} -c $< -o ${<:.cpp=.o}

all: 		${NAME}

${NAME}:	${OBJS}
			${COMPILER} ${OBJS} -o ${NAME} ${LIBRARIES}

clean:
			${RM} ${OBJS}

fclean: 	clean
			${RM} ${NAME}

re:
			make fclean
			make

.PHONY: 	all clean fclean re
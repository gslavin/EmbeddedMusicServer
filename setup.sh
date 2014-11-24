#!/bins/sh
FIFO_NAME='music_commands'
if [ ! -e $FIFO_NAME ]; then
    mkfifo $FIFO_NAME || exit 1
fi

#set fifo path
sed -i "/#define FIFO_PATH/"'c \'"#define FIFO_PATH \"${PWD}/$FIFO_NAME\"" $PWD/SoundRequestServer/sound_server.h

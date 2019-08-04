FROM debian:buster

RUN apt update && \
    apt install -y -t testing \
    gcc-8-multilib xutils-dev make gcc-8-multilib-x86-64-linux-gnux32 gcc

#VOLUME ["/os452"]

RUN echo "cd /os452 && make clean && make depend && make" > ./entrypoint.sh && \
    chmod +x ./entrypoint.sh
    
CMD ./entrypoint.sh

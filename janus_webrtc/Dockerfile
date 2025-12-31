FROM alpine

COPY dependencies.sh /usr/local/bin/dependencies.sh
RUN chmod +x /usr/local/bin/dependencies.sh
RUN /usr/local/bin/dependencies.sh

WORKDIR /home/
RUN git clone https://github.com/meetecho/janus-gateway.git

WORKDIR /home/janus-gateway
RUN ./autogen.sh
RUN ./configure --prefix=/opt/janus
RUN make
RUN make install
RUN make configs

CMD ["/opt/janus/bin/janus"]
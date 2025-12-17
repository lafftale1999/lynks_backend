FROM alpine

COPY dependencies.sh /usr/local/bin/dependencies.sh
RUN chmod +x /usr/local/bin/dependencies.sh

RUN /usr/local/bin/dependencies.sh

# pull and generate files
WORKDIR /home/usr/
RUN git clone https://github.com/meetecho/janus-gateway.git

WORKDIR /home/usr/janus-gateway

RUN ./autogen.sh

CMD ["sh"]
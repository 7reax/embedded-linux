CFLAGS = -O2 -Wall
BINS = pipe_throughput chat mq_throughput dispatcher client_time client_date

all: $(BINS)

dispatcher: dispatcher.c mq_proto.h
	$(CC) $(CFLAGS) -o $@ $<
client_time: client_time.c mq_proto.h
	$(CC) $(CFLAGS) -o $@ $<
client_date: client_date.c mq_proto.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(BINS)
	rm -f /tmp/chat_a2b /tmp/chat_b2a

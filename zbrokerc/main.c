#include<stdlib.h>
#include<stdio.h>
#include<zmq.h>

struct rec_data
{
	int id;
	int type;
	float data[16384];
};

struct rec_data* decode_z(char* msg);

struct rec_data* decode_z(char* msg)
{
	if (msg != NULL)
	{
		struct rec_data* result = malloc(sizeof(struct rec_data));
		result->id = (msg[26] - 48) * 10 + (msg[27] - 48);
		result->type = (msg[13] - 48) * 10 + (msg[14] - 48);
		char* data = result->data;
		for (int i = 4; i < 65537; i += 4)
		{
			for (int j = 0; j < 4; j++)
				*data++ = msg[48 + i - j];
		}
		return result;
	}
	return NULL;

}

void main(void)
{
	int major, minor, patch;
	zmq_version(&major, &minor, &patch);
	printf("hello. Major: %d, Minor: %d; Patch: %d", major,minor,patch);
	void* context = zmq_ctx_new();
	void* subscriber = zmq_socket(context, ZMQ_SUB);
	int rc = zmq_connect(subscriber, "tcp://localhost:5558");
	const char* filter = "";
	int rf = zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, filter, strlen(filter));
	
	while (1)
	{
		zmq_msg_t reply;
		zmq_msg_init(&reply);
		zmq_msg_recv(&reply, subscriber, 0);
		/*char buff[256];
		zmq_recv(subscriber, buff, 256, 0);*/
		int length = zmq_msg_size(&reply);
		
		char* value = malloc(length);
		
		memcpy(value, zmq_msg_data(&reply), length);
		
		zmq_msg_close(&reply);

		struct rec_data* r = decode_z(value);

		printf("id: %d, type: %d [0] = %f\tsize:%d\n", r->id, r->type, r->data[0], length);
		free(value);
		free(r);
	}

	zmq_close(subscriber);
	zmq_ctx_destroy(context);
	return 0;
}


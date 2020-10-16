#include <stdio.h>
#include <string.h>
#include <ulfius.h>
#include "../../include/auxilary.h"
#include "../../include/json.h"
#include "../../include/client/client.h"
#include "../../include/client/endpoints.h"

void get_blockchain_sender_transactions(char *address){
    json_t *blockchain;
    block_t block;
    int i,res;
	struct _u_request request;
    struct _u_response response;
	json_error_t json_error;
	size_t index;
	json_t *block_json;
	res = send_http_request(&request, &response, CLIENT_DEFAULT_IP_ADDRESS, 55000,
    ENDPOINT_BLOCKCHAIN_RETRIEVE_HTTP_METHOD, ENDPOINT_BLOCKCHAIN_RETRIEVE_URL_PATH, NULL, 100);
    if(res == U_OK && response.status == 200){
        if (!(blockchain = ulfius_get_json_body_response(&response, &json_error))){
            fprintf(stderr, "\n[ERROR] get_blockchain_sender_transactions/ulfius_get_json_body_response (%s): %s\n", request.http_url, json_error.text);
        }
    }
	json_array_foreach(blockchain, index, block_json){
		block = json_destruct_block(block_json);
		for (i=0; i< block.transaction_count; i++ ){
			if(!strcmp(address,block.transactions[i].sender)){
				printf("\nSender: %s\n",block.transactions[i].sender);
				printf("Receiver: %s\n",block.transactions[i].receiver);
				printf("Timestamp: %lu\n",block.transactions[i].timestamp);
				printf("Amount: %lf\n",block.transactions[i].amount);
			}
		}
	}

}

void get_blockchain_receiver_transactions(char *address){
    json_t *blockchain;
    block_t block;
    int i,res;
	struct _u_request request;
    struct _u_response response;
	json_error_t json_error;
	size_t index;
	json_t *block_json;

	res = send_http_request(&request, &response, CLIENT_DEFAULT_IP_ADDRESS, 55000,
    ENDPOINT_BLOCKCHAIN_RETRIEVE_HTTP_METHOD, ENDPOINT_BLOCKCHAIN_RETRIEVE_URL_PATH, NULL, 100);
// retrieve blockchain from response
    if(res == U_OK && response.status == 200){
        if (!(blockchain = ulfius_get_json_body_response(&response, &json_error))){
            fprintf(stderr, "\n[ERROR] send_http_request/ulfius_get_json_body_response (%s): %s\n", request.http_url, json_error.text);
        }
    }
	json_array_foreach(blockchain, index, block_json){
		block = json_destruct_block(block_json);
		for (i=0; i< block.transaction_count; i++ ){
			if(!strcmp(address,block.transactions[i].receiver)){
				printf("\nSender: %s\n",block.transactions[i].sender);
				printf("Receiver: %s\n",block.transactions[i].receiver);
				printf("Timestamp: %lu\n",block.transactions[i].timestamp);
				printf("Amount: %lf\n",block.transactions[i].amount);
			}
		}
	}

}

void get_block_id(int block_id){
 	json_t *blockchain;
 	block_t block;
 	int i,j,res;
	struct _u_request request;
    struct _u_response response;
	json_error_t json_error;
	size_t index;
	json_t *block_json;
	res = send_http_request(&request, &response, CLIENT_DEFAULT_IP_ADDRESS, 55000,
    ENDPOINT_BLOCKCHAIN_RETRIEVE_HTTP_METHOD, ENDPOINT_BLOCKCHAIN_RETRIEVE_URL_PATH, NULL, 100);

	// If the response is valid, retrieve the JSON node array from response's body
	if(res == U_OK && response.status == 200){
        if (!(blockchain = ulfius_get_json_body_response(&response, &json_error))){
            fprintf(stderr, "\n[ERROR] get_block_id/ulfius_get_json_body_response (%s): %s\n", request.http_url, json_error.text);
        }
    }

	json_array_foreach(blockchain, index, block_json){
		block = json_destruct_block(block_json);
		for (i=0; i< json_array_size(blockchain); i++ ){
			if(block_id == block.id){
				printf("\nBlock ID: %d\n",block.id);
				printf("Nonce: %d\n",block.nonce);
				printf("Timestamp: %lu\n",block.timestamp);
				printf("Previous: %s\n",block.previous);
				printf("Hash: %s\n\n\nTransactions:\n\n",block.hash);


				for (j=0; j< block.transaction_count; j++ ){
					printf("Sender: %s\n",block.transactions[j].sender);
					printf("Receiver: %s\n",block.transactions[j].receiver);
					printf("Timestamp: %lu\n",block.transactions[j].timestamp);
					printf("Amount: %lf\n\n",block.transactions[j].amount);
				}
			break;
			}	
		}
	}

}

int main(int argc, const char **argv){

    int input,block_id;
    char address[34];

    fprintf(stdout, "\n Choose to get information for:\n 1.Sender\n 2.Receiver\n 3.BlockID\n\n  ");
    printf("Choice: ");
    scanf("%d",&input);

    while (input < 1 || input > 3) {
        printf("Please provide again a valid option:\n 1.Sender\n 2.Receiver\n 3.BlockID\n\n");
        printf("Choice: ");
		scanf("%d",&input);
    }
    printf("The number you entered is %d \n\n", input);
	while ((getchar()) != '\n');
    switch (input)
    {
	case 1:
	   printf("Please provide a valid 32 hex digit sender address:\n");
	   fgets(address, sizeof(address), stdin);
	   address[strlen(address) - 1] = '\0';
	   get_blockchain_sender_transactions(address);
           break;
	case 2: 
	   printf("Please provide a valid 32 hex digit receiver address:\n");
	   fgets(address, sizeof(address), stdin);
	   address[strlen(address) - 1] = '\0';
	   get_blockchain_receiver_transactions(address);
           break;
	case 3: 
	   printf("Please provide a valid block ID:\n");
	   scanf("%d",&block_id);

		while (block_id < 1 || block_id > 100000) {
	 		printf("Please provide a valid block ID:\n");
    		scanf("%d",&block_id);
	    }
	   get_block_id(block_id); 
    }

    return 0;
}

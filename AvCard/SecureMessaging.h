#pragma once
#include "cardobject.h"
#include "belt.h"
#include "bign.h"
#include "rng.h"
#include "password.h"
#include "helpers.h"
#include "ProtocolAgent.h"
#include "point.h" 

#define SM_KEY_SHIFT_LENGTH			BELT_KEY_LENGHT
#define SYNCHRO_LENGTH				BELT_SYNCHRO_LENGHT
#define IMITO_LENGTH				BELT_MAC_LENGHT
#define PACE_GENPOINT_PROTOTYPE_SIZE	32
#define PACE_KEY_EFFICIENT_LENGHT		32
#define PACE_REQUEST_1_SIZE		PACE_GENPOINT_PROTOTYPE_SIZE/2
#define PACE_REQUEST_2_SIZE		BIGN_POINT_LENGHT+IMITO_LENGTH
#define PACE_RESPONSE_1_SIZE	PACE_GENPOINT_PROTOTYPE_SIZE/2 + BIGN_PUBKEY_LENGHT
#define PACE_RESPONSE_2_SIZE	IMITO_LENGTH


class SecureMessaging :
	public CardObject, 
	public ProtocolAgent
{
protected:
	Password* pwd;
	RNG* rng;
	//BELT_KEY_LENGHT
	byte* keyMain;
	byte* keyEncr;
	byte* keyMAC;
	int paceStep;
	void refreshKeys();
	void refreshSubKey();
	void rndNumberUnderQ(byte* to);

public:
	string getProtocolName(){
		return "SM";
	}
	//[ cmd :< MSE : Set  AT >] 
	bool setTemplate(Password* pwd);
	
	//shift ~ BELT_KEY_LENGHT(32)
	void update(byte* shift);
	void wrap(byte* message,int length, byte* to );
	void unwrap(byte* message,int length, byte* to);

	void deactivate(){
		CardObject::deactivate();
		delete[BIGN_POINT_LENGHT] keyMain;
		delete[BIGN_POINT_LENGHT] keyEncr;
		delete[BIGN_POINT_LENGHT] keyMAC;
		delete pwd;
	}
	
	
	SecureMessaging(){
		paceStep=0;
		byte* keyMain=new byte[BIGN_POINT_LENGHT];
		byte* keyEncr=new byte[BIGN_POINT_LENGHT];
		byte* keyMAC=new byte[BIGN_POINT_LENGHT];
	}
	~SecureMessaging(){
		delete[BIGN_POINT_LENGHT] keyMain;
		delete[BIGN_POINT_LENGHT] keyEncr;
		delete[BIGN_POINT_LENGHT] keyMAC;

	}
};

class SMResponser:public SecureMessaging{
private:
	//32 byte
	byte* myPACEkeyPart;

	//[ cmd :< MSE : General    Authenticate >]
    void response1 (byte* request1);
	//[ cmd :< MSE : General    Authenticate >] 
	void response2 (byte* request2);
	
	

public:
	void receive(byte* message, int size){
		try{
			switch(paceStep){
				case 0:
					if(size!=PACE_REQUEST_1_SIZE)
						throw exception("pace packet error");
					response1(message);
		
					break;
				case 1:
					if(size!=PACE_REQUEST_2_SIZE)
						throw exception("pace packet error");
					response2(message);
					break;
			}
			paceStep++;
		}catch(exception ex){
			deactivate();
			paceStep=0;
		}
	}
	SMResponser(void){
		paceStep=0;
		myPACEkeyPart=new byte[PACE_KEY_EFFICIENT_LENGHT];
		
	}
	~SMResponser(void){
		delete[PACE_KEY_EFFICIENT_LENGHT] myPACEkeyPart;
		
	}
};

class SMRequester:public SecureMessaging{
private:
	byte* fullPointProto;
	//for check
	byte* myPoint;

	void request1 ();
	//[ cmd :< MSE : General    Authenticate >]
    void request2 (byte* response1);

	void check(byte* response2);

public:
	void initiate(){
		request1();
		paceStep=1;
	}
	void receive(byte* message, int size){
		try{
			switch(paceStep){
				case 1:
					if(size!=PACE_RESPONSE_1_SIZE)
						throw exception("pace packet error");
					request2(message);
					break;
				case 2:
					if(size!=PACE_RESPONSE_2_SIZE)
						throw exception("pace packet error");
					check(message);
					break;
			}
			paceStep++;
		}catch(exception ex){
			deactivate();
			paceStep=0;
		}
	}

	SMRequester(void){
		fullPointProto=new byte[PACE_GENPOINT_PROTOTYPE_SIZE];
		myPoint=new byte[BIGN_POINT_LENGHT];
	}
	~SMRequester(void){
		delete[PACE_GENPOINT_PROTOTYPE_SIZE] fullPointProto;
		delete[BIGN_POINT_LENGHT] myPoint;
		
	}
};


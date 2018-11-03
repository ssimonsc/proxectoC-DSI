//Libraries for DDS
#include "HelloWorldSubscriber.h"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/SubscriberAttributes.h>
#include <fastrtps/subscriber/Subscriber.h>
#include <fastrtps/Domain.h>
#include <fastrtps/utils/eClock.h>

//Libraries for Navio
#include <unistd.h>
#include "Navio2/PWM.h"
#include "Navio+/RCOutput_Navio.h"
#include "Navio2/RCOutput_Navio2.h"
#include "Common/Util.h"
#include <unistd.h>
#include <memory>

#define SERVO_MIN 1 /*mS*/
#define SERVO_MAX 2 /*mS*/

#define PWM_OUTPUT_RIGHT 1
#define PWM_OUTPUT_LEFT  2


using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;
using namespace Navio;

HelloWorldSubscriber::HelloWorldSubscriber():mp_participant(nullptr),
mp_subscriber(nullptr)
{
}

//Initialize DDS
bool HelloWorldSubscriber::init()
{
    ParticipantAttributes PParam;
    PParam.rtps.defaultSendPort = 10043;
    PParam.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
    PParam.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
    PParam.rtps.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    PParam.rtps.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    PParam.rtps.builtin.domainId = 0;
    PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
    PParam.rtps.setName("Participant_sub");
    mp_participant = Domain::createParticipant(PParam);
    if(mp_participant==nullptr)
        return false;

    //REGISTER THE TYPE

    Domain::registerType(mp_participant,&m_type);
    //CREATE THE SUBSCRIBER
    SubscriberAttributes Rparam;
    Rparam.topic.topicKind = NO_KEY;
    Rparam.topic.topicDataType = "HelloWorld";
    Rparam.topic.topicName = "HelloWorldTopic";
    Rparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Rparam.topic.historyQos.depth = 30;
    Rparam.topic.resourceLimitsQos.max_samples = 50;
    Rparam.topic.resourceLimitsQos.allocated_samples = 20;
    Rparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    Rparam.qos.m_durability.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    mp_subscriber = Domain::createSubscriber(mp_participant,Rparam,(SubscriberListener*)&m_listener);

    if(mp_subscriber == nullptr)
        return false;


    return true;
}

HelloWorldSubscriber::~HelloWorldSubscriber() {
    // TODO Auto-generated destructor stub
    Domain::removeParticipant(mp_participant);
}

void HelloWorldSubscriber::SubListener::onSubscriptionMatched(Subscriber* /*sub*/,MatchingInfo& info)
{
    if(info.status == MATCHED_MATCHING)
        std::cout << "Subscriber matched"<<std::endl;
    else
        std::cout << "Subscriber unmatched"<<std::endl;
}

std::unique_ptr <RCOutput> get_rcout()
{
    if (get_navio_version() == NAVIO2)
    {
	std::cout << "Ola Caracola"<<std::endl;
        auto ptr = std::unique_ptr <RCOutput>{ new RCOutput_Navio2()};
        return ptr;
    }// else
   // {
    //    auto ptr = std::unique_ptr <RCOutput>{ new RCOutput_Navio() };
  //      return ptr;
//    }

}

void HelloWorldSubscriber::SubListener::onNewDataMessage(Subscriber* sub)
{
    float servoRight;
    float servoLeft;
    auto pwm = get_rcout(); 
    if(sub->takeNextData((void*)&m_Hello, &m_info))
    {
        if(m_info.sampleKind == ALIVE)
        {
            // Print your structure data here.
	    if(m_Hello.direction() !=1523 ){
		servoRight = (m_Hello.forward()-1099)*0.0009;
		servoLeft = servoRight;
		//move to the left
		if(m_Hello.direction()<1523){
			servoLeft = servoLeft + (1523-m_Hello.direction())*0.0006;
		}
		//move to the right
		if(m_Hello.direction()>1523){
			servoRight  = servoRight + (m_Hello.direction()-1523)*0.0006;
		}
	    }

	    //send data to engines
	    pwm->set_duty_cycle(PWM_OUTPUT_RIGHT, servoRight);
	    pwm->set_duty_cycle(PWM_OUTPUT_LEFT, servoLeft);

            std::cout << "Message "<<m_Hello.forward()<< " "<< m_Hello.direction()<< " RECEIVED"<<std::endl;
        }
    }

}


int HelloWorldSubscriber::run()
{
    std::cout << "Subscriber running. Please press enter to stop the Subscriber" << std::endl;
    std::cin.ignore();
        auto pwm = get_rcout();

        if (check_apm()) {
            return 1;
        }

        if( !(pwm->initialize(PWM_OUTPUT_RIGHT)) ) {
            return 1;
        }

        if( !(pwm->initialize(PWM_OUTPUT_LEFT)) ) {
            return 1;
        }

        pwm->set_frequency(PWM_OUTPUT_RIGHT, 50);
        pwm->set_frequency(PWM_OUTPUT_LEFT, 50);


        if ( !(pwm->enable(PWM_OUTPUT_RIGHT)) ) {
            return 1;
        }

        if ( !(pwm->enable(PWM_OUTPUT_LEFT)) ) {
            return 1;
	}

        for(int i=0; i<4; i++){
                pwm->set_duty_cycle(PWM_OUTPUT_RIGHT, SERVO_MIN);
                pwm->set_duty_cycle(PWM_OUTPUT_LEFT, SERVO_MIN);
                sleep(1);
        }

    return 0;
}


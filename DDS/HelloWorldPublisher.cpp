//Libraries for DDS
#include "HelloWorldPublisher.h"
#include <fastrtps/participant/Participant.h>
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastrtps/publisher/Publisher.h>
#include <fastrtps/Domain.h>
#include <fastrtps/utils/eClock.h>
#include <unistd.h>
#include <cstdio>
//Navio libraries
#include </home/pi/Navio2/C++/Navio/Navio2/RCInput_Navio2.h>
#include </home/pi/Navio2/C++/Navio/Navio+/RCInput_Navio.h>
#include </home/pi/Navio2/C++/Navio/Common/Util.h>
#include <memory>

#define READ_FAILED -1

using namespace eprosima::fastrtps;
using namespace eprosima::fastrtps::rtps;

HelloWorldPublisher::HelloWorldPublisher():mp_participant(nullptr),
mp_publisher(nullptr)
{


}

bool HelloWorldPublisher::init()
{
    m_Hello.forward(1099);
    m_Hello.direction(1523);
    m_Hello.emergencyStop(964);
    ParticipantAttributes PParam;
    PParam.rtps.defaultSendPort = 11511;
    PParam.rtps.use_IP6_to_send = true;
    PParam.rtps.builtin.use_SIMPLE_RTPSParticipantDiscoveryProtocol = true;
    PParam.rtps.builtin.use_SIMPLE_EndpointDiscoveryProtocol = true;
    PParam.rtps.builtin.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = true;
    PParam.rtps.builtin.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = true;
    PParam.rtps.builtin.domainId = 0;
    PParam.rtps.builtin.leaseDuration = c_TimeInfinite;
    PParam.rtps.setName("Participant_pub");
    mp_participant = Domain::createParticipant(PParam);

    if(mp_participant==nullptr)
        return false;
    //REGISTER THE TYPE

    Domain::registerType(mp_participant,&m_type);

    //CREATE THE PUBLISHER
    PublisherAttributes Wparam;
    Wparam.topic.topicKind = NO_KEY;
    Wparam.topic.topicDataType = "HelloWorld";
    Wparam.topic.topicName = "HelloWorldTopic";
    Wparam.topic.historyQos.kind = KEEP_LAST_HISTORY_QOS;
    Wparam.topic.historyQos.depth = 30;
    Wparam.topic.resourceLimitsQos.max_samples = 50;
    Wparam.topic.resourceLimitsQos.allocated_samples = 20;
    Wparam.times.heartbeatPeriod.seconds = 2;
    Wparam.times.heartbeatPeriod.fraction = 200*1000*1000;
    Wparam.qos.m_reliability.kind = RELIABLE_RELIABILITY_QOS;
    mp_publisher = Domain::createPublisher(mp_participant,Wparam,(PublisherListener*)&m_listener);
    if(mp_publisher == nullptr)
        return false;

    return true;

}

HelloWorldPublisher::~HelloWorldPublisher()
{
    // TODO Auto-generated destructor stub
    Domain::removeParticipant(mp_participant);
}

std::unique_ptr <RCInput> get_rcin()
{
    if (get_navio_version() == NAVIO2)
    {
	std::cout << "Ola Caracola"<<std::endl;
        auto ptr = std::unique_ptr <RCInput>{ new RCInput_Navio2()};
        return ptr;
    }// else

    //{
       // auto ptr = std::unique_ptr <RCInput>{ new RCInput_Navio()};
     //   return ptr;
   // }

}

void HelloWorldPublisher::PubListener::onPublicationMatched(Publisher* /*pub*/,MatchingInfo& info)
{
    if(info.status == MATCHED_MATCHING)
    {
        std::cout << "Publisher matched"<<std::endl;
    }
    else
    {
        std::cout << "Publisher unmatched"<<std::endl;
    }
}


int HelloWorldPublisher::run(){

        if (check_apm()) {
        return 1;
    }


   auto rcin = get_rcin();

    rcin->initialize();
    printf("Just inicialized the RC");

	
    while(true){
	int forward = rcin-> read(1);
        if(forward == READ_FAILED) return EXIT_FAILURE;
        printf("Reading forward %i \n", forward);
sleep(1);
        int direction = rcin-> read(3);
        if (direction == READ_FAILED) return EXIT_FAILURE;
        printf("Reading direction %i \n", direction);
sleep(1);        
int emergencyStop = rcin -> read(4);
        if(emergencyStop == READ_FAILED) return EXIT_FAILURE;
        printf("Reading emergency stop %i \n", emergencyStop);
        m_Hello.forward(forward);
        m_Hello.direction(direction);
	if(emergencyStop == 964) m_Hello.emergencyStop(1);
        else m_Hello.emergencyStop(0);
        mp_publisher->write((void*)&m_Hello);
        }

	sleep(1);
}


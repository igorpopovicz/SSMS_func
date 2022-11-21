#include <stdio.h>
#include "../inc/snmpGet.h"
#include <pqxx/pqxx>
#include <iostream>
#include <ctime>


using namespace std;
using namespace pqxx;

#define DB "dbname = teste user = postgres password = docker \
  hostaddr = 127.0.0.1 port = 5432"
#define TABLE "loc_status_smf"

int greenInt;
int redInt;
int cycleTimer;
int yellowInt;
int syncTimer;
int nextFaseInt;
int pedestrianInt;
int overlapGreenInt;
int overlapYellowInt;
int overlapRedInt;
int overlapPedGreenInt;
int overlapPedYellowInt;
int overlapPedRedInt;
int nextFase[17];
int greenPhase[17];
int yellowPhase[17];
int redPhase[17];
int overlapGreenPhase[17];
int overlapYellowPhase[17];
int overlapRedPhase[17];
int overlapPedGreenPhase[17];
int overlapPedYellowPhase[17];
int overlapPedRedPhase[17];
int pedestrianPhase[17];
int phaseActive[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int phaseActiveDecimal;

bool semex = 0;
int  unity;

connection C(DB);
nontransaction N(C);

class Log
{
public:
    const int LogLevelError   = 0;
    const int LogLevelWarning = 1;
    const int LogLevelInfo    = 2;

private:
    int m_LogLevel = LogLevelInfo;

public:
    void SetLevel(int level)
    {
        m_LogLevel = level;
    }

    void Error(const char* message)
    {
        if(m_LogLevel >= LogLevelError)
            std::cout << "[ERROR]:" << message << std::endl;
    }

    void Warn(const char* message)
    {
        if(m_LogLevel >= LogLevelWarning)
            std::cout << "[WARNING]:" << message << std::endl;
    }

    void Info(const char* message)
    {
        if(m_LogLevel >= LogLevelInfo)
            std::cout << "[INFO]:" << message << std::endl;
    }
};


void update(const std::string &__table, const std::string &__column, int value, int id)
{
  
  char buff[100];
  if(value == 999)
    sprintf(buff, "UPDATE %s set %s=NULL where unity=%d", __table.c_str(), __column.c_str(), id);
  else
    sprintf(buff, "UPDATE %s set %s = %d where unity=%d", __table.c_str(), __column.c_str(), value, id);

  N.exec(buff);
}

void updatechar(const std::string &__table, const std::string &__column, const std::string &__value, int id)
{
  char buff[255];
  sprintf(buff, "UPDATE %s set %s = '%s' where unity=%d", __table.c_str(), __column.c_str(), __value.c_str(), id);

  N.exec(buff);
}

void insert(const std::string &__table, const std::string &__column, int value, int id)
{
  char buff[100];
  sprintf(buff, "INSERT INTO %s (ID, %s)VALUES (%d, %d);", __table.c_str(), __column.c_str(), id, value);
  
  N.exec(buff);
}

void updateTime(const std::string &__table, int id)
{
  char buff[100];
  sprintf(buff, "UPDATE %s set last_com=now() where unity=%d", __table.c_str(), id);

  N.exec(buff);
}

void updateNULL(const std::string &__table, int id)
{
  updatechar(TABLE, "next_fase", " ", id);
  for(int i = 1; i < 17; i++)
  {
    char faseBuff[6];
    sprintf(faseBuff, "fase%d", i);
    update(TABLE, faseBuff, 1, id);
  }

  for(int i = 1; i < 17; i++)
  {
    char faseBuff[7];
    sprintf(faseBuff, "fasep%d", i);
    update(TABLE, faseBuff, 1, id);
  }

  for(int i = 1; i < 17; i++)
  {
    char faseBuff[15];
    sprintf(faseBuff, "overlap%d", i);
    update(TABLE, faseBuff, 1, unity);
  }

  update(TABLE, "tp_ciclo", 0, id);
  update(TABLE, "tp_sync", 0, id);
}

void getPhases(const std::string &__table)
{

  char buff[100];
  sprintf(buff, "SELECT * from %s", __table.c_str());

  result R( N.exec( buff ));

  for(result::const_iterator c = R.begin(); c != R.end(); ++c)
  {
    greenInt = 0;
    yellowInt = 0;
    redInt = 0;
    cycleTimer = 0;
    syncTimer = 0;
    nextFaseInt = 0;
    pedestrianInt = 0;

    overlapGreenInt = 0;
    overlapYellowInt = 0;
    overlapRedInt = 0;

    overlapPedGreenInt = 0;
    overlapPedYellowInt = 0;
    overlapPedRedInt = 0;
    
    semex = c[1].as<bool>();

    if(semex)
    {
      unity = c[5].as<int>();

      char ipS[30];
      char ubuff[100];
      if(unity < 255)
        sprintf(ipS, "192.168.%d.23:%d", unity, 161);
      else
      {
        updateNULL(TABLE, unity);
        continue;
      }

      greenInt =  snmpGet(".1.3.6.1.4.1.1206.4.2.1.1.4.1.4.1", ipS);
      if(greenInt < 0)
      {
        updateNULL(TABLE, unity);
        continue;
      }
        
      yellowInt =  snmpGet(".1.3.6.1.4.1.1206.4.2.1.1.4.1.3.1", ipS);
      if(yellowInt < 0)
      {
        updateNULL(TABLE, unity);
        continue;
      }

      redInt =  snmpGet(".1.3.6.1.4.1.1206.4.2.1.1.4.1.2.1", ipS);
      if(redInt < 0)
      {
        updateNULL(TABLE, unity);
        continue;
      }
      
      overlapRedInt =  snmpGet(".1.3.6.1.4.1.1206.4.2.1.9.4.1.2.1", ipS);  
      if(overlapRedInt < 0)
      {
        updateNULL(TABLE, unity);
        continue;
      }
      
      overlapYellowInt =  snmpGet(".1.3.6.1.4.1.1206.4.2.1.9.4.1.3.1", ipS);   
      if(overlapYellowInt < 0)
      {
        updateNULL(TABLE, unity);
        continue;
      }
      
      overlapGreenInt  =  snmpGet(".1.3.6.1.4.1.1206.4.2.1.9.4.1.4.1", ipS);
      if(overlapGreenInt < 0)
      {
        updateNULL(TABLE, unity);
        continue;
      }
      
      overlapPedRedInt = snmpGet(".1.3.6.1.4.1.1206.4.2.1.9.4.1.2.1", ipS);
      if(overlapPedRedInt < 0)
      {
        updateNULL(TABLE, unity);
        continue;
      }
      
      overlapPedYellowInt =  snmpGet(".1.3.6.1.4.1.1206.4.2.1.9.4.1.3.1", ipS);  
      if(overlapPedYellowInt < 0)
      {
        updateNULL(TABLE, unity);
        continue;
      }
      
      overlapPedGreenInt  =  snmpGet(".1.3.6.1.4.1.1206.4.2.1.9.4.1.4.1", ipS);      
      if(overlapPedGreenInt < 0)
      {
        updateNULL(TABLE, unity);
        continue;
      }
      
      cycleTimer = snmpGet(".1.3.6.1.4.1.1206.4.2.1.4.12.0", ipS);      
      if(cycleTimer < 0)
      {
        updateNULL(TABLE, unity);
        continue;
      }
      
      syncTimer  = snmpGet(".1.3.6.1.4.1.1206.4.2.1.4.13.0", ipS);     
      if(syncTimer < 0)
      {
        updateNULL(TABLE, unity);
        continue;
      }
      
      nextFaseInt   = snmpGet(".1.3.6.1.4.1.1206.4.2.1.1.4.1.11.1", ipS);
      if(nextFaseInt < 0)
      {
        updateNULL(TABLE, unity);
        continue;
      }
      
      pedestrianInt = snmpGet(".1.3.6.1.4.1.1206.4.2.1.1.4.1.7.1", ipS);
      if(pedestrianInt < 0)
      {
        updateNULL(TABLE, unity);
        continue;
      }
      
      string nextFaseString;
      for(int i = 0; i<=15; i++)
      {
        nextFase[i] = (nextFaseInt>>i)&1;
        if(nextFase[i] == 1)
        {
          char nextFaseBuff[10];
          sprintf(nextFaseBuff, "%d,", i+1);
          nextFaseString.append(nextFaseBuff);
        }
      }
      updatechar(TABLE, "next_fase", nextFaseString, unity);

      for(int i = 0; i<15; i++)
      {
        char oidBuff[38];
        sprintf(oidBuff, ".1.3.6.1.4.1.1206.4.2.1.1.2.1.6.%d", i+1);
        if(int x = snmpGet(oidBuff, ipS) > 1)
          phaseActive[i] = 1;
        else
          phaseActive[i] = 0;
      }

      for(int i = 0; i<=15; i++)
      {
        char faseBuff[6];
        sprintf(faseBuff, "fase%d", i+1);
        if(phaseActive[i] == 0)
          update(TABLE, faseBuff, 999, unity);
      }

      for(int i = 15; i>=0; i--)
      {
        phaseActiveDecimal = phaseActiveDecimal * 2 + phaseActive[i];
      }

      update(TABLE, "fase_ativa", phaseActiveDecimal, unity);
      phaseActiveDecimal = 0;

      for(int i = 0; i<=15; i++)
      {
        greenPhase[i] = (greenInt>>i)&1;
        if(greenPhase[i] == 1)
        {
          char faseBuff[6];
          sprintf(faseBuff, "fase%d", i+1);
          update(TABLE, faseBuff, 0, unity);
        }
      }

      for(int i = 0; i<=15; i++)
      {
        yellowPhase[i] = (yellowInt>>i)&1;
        if(yellowPhase[i] == 1)
        {
          char faseBuff[6];
          sprintf(faseBuff, "fase%d", i+1);
          update(TABLE, faseBuff, 1, unity);
        }
      }
      
      for(int i = 0; i<=15; i++)
      {
        redPhase[i] = (redInt>>i)&1;
        if(redPhase[i] == 1)
        {
          char faseBuff[6];
          sprintf(faseBuff, "fase%d", i+1);
          update(TABLE, faseBuff, 2, unity);
        }
      }

      //Pedestrian
      for(int i = 0; i<=15; i++)
      {
        pedestrianPhase[i] = (pedestrianInt>>i)&1;
        char faseBuff[7];
        sprintf(faseBuff, "fasep%d", i+1);
        update(TABLE, faseBuff, !pedestrianPhase[i], unity);
      }

      //Overlap
      for(int i = 0; i<=15; i++)
      {
        overlapGreenPhase[i] = (overlapGreenInt>>i)&1;
        if(overlapGreenPhase[i] == 1)
        {
          char faseBuff[15];
          sprintf(faseBuff, "overlap%d", i+1);
          update(TABLE, faseBuff, 0, unity);
        }
      }

      for(int i = 0; i<=15; i++)
      {
        overlapYellowPhase[i] = (overlapYellowInt>>i)&1;
        if(overlapYellowPhase[i] == 1)
        {
          char faseBuff[15];
          sprintf(faseBuff, "overlap%d", i+1);
          update(TABLE, faseBuff, 1, unity);
        }
      }
      
      for(int i = 0; i<=15; i++)
      {
        overlapRedPhase[i] = (overlapRedInt>>i)&1;
        if(overlapRedPhase[i] == 1)
        {
          char faseBuff[15];
          sprintf(faseBuff, "overlap%d", i+1);
          update(TABLE, faseBuff, 2, unity);
        }
      }
        
      update(TABLE, "tp_ciclo", cycleTimer, unity);
      update(TABLE, "tp_sync", syncTimer, unity);
      updateTime(TABLE, unity);
    }
  }
}

int main()
{
 while (1)
  getPhases(TABLE);
}
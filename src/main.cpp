#include <stdio.h>
#include "../inc/snmpGet.h"
#include <pqxx/pqxx>
#include <iostream>

using namespace std;
using namespace pqxx;

#define DB "dbname = ssms user = ssms password = ssms \
  hostaddr = 127.0.0.1 port = 5432"
#define TABLE "loc_status_smf"  

int greenInt;
int yellowInt;
int redInt;
int cycleTimer;
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
int phaseActiveDecimal = 0;

bool semex = 0;
int  unity;

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
  Log log;

  log.SetLevel(log.LogLevelError);

  connection C(DB);

  if (C.is_open()){
    log.Info("Banco de dados aberto com sucesso");
  }
  else {
    log.Error("Não foi possível abrir o banco de dados");
    return;
  }
  
  char buff[100];
  if(value == 999)
    sprintf(buff, "UPDATE %s set %s=NULL where unity=%d", __table.c_str(), __column.c_str(), id);
  else
    sprintf(buff, "UPDATE %s set %s = %d where unity=%d", __table.c_str(), __column.c_str(), value, id);

  nontransaction N(C);
  N.exec(buff);
  log.Info("UPDATE realizado");

  C.disconnect(); 
}

void updatechar(const std::string &__table, const std::string &__column, const std::string &__value, int id)
{
  Log log;

  log.SetLevel(log.LogLevelError);

  connection C(DB);

  if (C.is_open()){
    log.Info("Banco de dados aberto com sucesso");
  }
  else {
    log.Error("Não foi possível abrir o banco de dados");
    return;
  }
  
  char buff[255];
  sprintf(buff, "UPDATE %s set %s = '%s' where unity=%d", __table.c_str(), __column.c_str(), __value.c_str(), id);

  nontransaction N(C);
  N.exec(buff);
  log.Info("UPDATE realizado");

  C.disconnect(); 
}

void insert(const std::string &__table, const std::string &__column, int value, int id)
{
  Log log;

  log.SetLevel(log.LogLevelError);

  connection C(DB);

  if (C.is_open()){
    log.Info("Banco de dados aberto com sucesso");
  }
  else {
    log.Error("Não foi possível abrir o banco de dados");
    return;
  }
  
  char buff[100];
  sprintf(buff, "INSERT INTO %s (ID, %s)VALUES (%d, %d);", __table.c_str(), __column.c_str(), id, value);
  
  work W(C);
  W.exec(buff);
  W.commit();
  log.Info("INSERT realizado");

  C.disconnect(); 
}

void getPhases(const std::string &__table)
{
  Log log;

  log.SetLevel(log.LogLevelError);

  connection C(DB);

  if (C.is_open()){
    log.Info("Banco de dados aberto com sucesso");
  }
  else {
    log.Error("Não foi possível abrir o banco de dados");
    return;
  }
  
  char buff[100];
  sprintf(buff, "SELECT * from %s", __table.c_str());
  
  nontransaction N(C);

  result R( N.exec( buff ));

  for(result::const_iterator c = R.begin(); c != R.end(); ++c)
  {
    semex = c[1].as<bool>();

    if(semex)
    {
      unity = c[5].as<int>();

      char ipS[30];
      sprintf(ipS, "192.168.%d.23:%d", unity, 161);

      greenInt  =  snmpGet(".1.3.6.1.4.1.1206.4.2.1.1.4.1.4.1", ipS);
      yellowInt =  snmpGet(".1.3.6.1.4.1.1206.4.2.1.1.4.1.3.1", ipS);
      redInt    =  snmpGet(".1.3.6.1.4.1.1206.4.2.1.1.4.1.2.1", ipS);

      overlapRedInt    =  snmpGet(".1.3.6.1.4.1.1206.4.2.1.9.4.1.2.1", ipS);
      overlapYellowInt =  snmpGet(".1.3.6.1.4.1.1206.4.2.1.9.4.1.3.1", ipS);
      overlapGreenInt  =  snmpGet(".1.3.6.1.4.1.1206.4.2.1.9.4.1.4.1", ipS);

      overlapPedRedInt    =  snmpGet(".1.3.6.1.4.1.1206.4.2.1.9.4.1.2.1", ipS);
      overlapPedYellowInt =  snmpGet(".1.3.6.1.4.1.1206.4.2.1.9.4.1.3.1", ipS);
      overlapPedGreenInt  =  snmpGet(".1.3.6.1.4.1.1206.4.2.1.9.4.1.4.1", ipS);      

      cycleTimer = snmpGet(".1.3.6.1.4.1.1206.4.2.1.4.12.0", ipS);
      syncTimer  = snmpGet(".1.3.6.1.4.1.1206.4.2.1.4.13.0", ipS);
      nextFaseInt   = snmpGet(".1.3.6.1.4.1.1206.4.2.1.1.4.1.11.1", ipS);

      pedestrianInt = snmpGet(".1.3.6.1.4.1.1206.4.2.1.1.4.1.7.1", ipS);

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
    }
  }
}

int main()
{
 while (1)
  getPhases(TABLE);
}
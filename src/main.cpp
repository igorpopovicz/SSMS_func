#include <stdio.h>
#include "../inc/snmpGet.h"
#include <pqxx/pqxx>
#include <iostream>

using namespace std;
using namespace pqxx;

int greenInt;
int yellowInt;
int redInt;
int cycleTimer;
int syncTimer;
int nextFase;
int pedestrianInt;

int greenPhase[17];
int yellowPhase[17];
int redPhase[17];
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

  connection C("dbname = teste user = postgres password = docker \
  hostaddr = 127.0.0.1 port = 5432");

  if (C.is_open()){
    log.Info("Banco de dados aberto com sucesso");
  }
  else {
    log.Error("Não foi possível abrir o banco de dados");
    return;
  }
  
  char buff[100];
  sprintf(buff, "UPDATE %s set %s = %d where unity=%d", __table.c_str(), __column.c_str(), value, id);

  nontransaction N(C);
  N.exec(buff);
  log.Info("UPDATE realizado");

  C.disconnect(); 
}

void insert(const std::string &__table, const std::string &__column, int value, int id)
{
  Log log;

  log.SetLevel(log.LogLevelError);

  connection C("dbname = teste user = postgres password = docker \
  hostaddr = 127.0.0.1 port = 5432");

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

  connection C("dbname = teste user = postgres password = docker \
  hostaddr = 127.0.0.1 port = 5432");

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

      cycleTimer = snmpGet(".1.3.6.1.4.1.1206.4.2.1.4.12.0", ipS);
      syncTimer  = snmpGet(".1.3.6.1.4.1.1206.4.2.1.4.13.0", ipS);
      nextFase   = snmpGet(".1.3.6.1.4.1.1206.4.2.1.1.4.1.11.1", ipS);

      pedestrianInt = snmpGet(".1.3.6.1.4.1.1206.4.2.1.1.4.1.7.1", ipS);

      for(int i = 1; i<16; i++)
      {
        char oidBuff[38];
        sprintf(oidBuff, ".1.3.6.1.4.1.1206.4.2.1.1.2.1.6.%d", i);
        if(int x = snmpGet(oidBuff, ipS) > 1)
          phaseActive[i] = 1;
        else
          phaseActive[i] = 0;
      }

      for(int i = 15; i>0; i--)
      {
        phaseActiveDecimal = phaseActiveDecimal * 2 + phaseActive[i];
      }

      update("locstatussmf", "fase_ativa", phaseActiveDecimal, unity);
      phaseActiveDecimal = 0;

      for(int i = 1; i<16; i++)
      {
        greenPhase[i] = (greenInt>>i)&1;
        if(greenPhase[i] == 1)
        {
          char faseBuff[6];
          sprintf(faseBuff, "fase%d", i);
          update("locstatussmf", faseBuff, 0, unity);
        }
      }

      for(int i = 1; i<16; i++)
      {
        yellowPhase[i] = (yellowInt>>i)&1;
        if(yellowPhase[i] == 1)
        {
          char faseBuff[6];
          sprintf(faseBuff, "fase%d", i);
          update("locstatussmf", faseBuff, 1, unity);
        }
      }
      
      for(int i = 1; i<16; i++)
      {
        redPhase[i] = (redInt>>i)&1;
        if(redPhase[i] == 1)
        {
          char faseBuff[6];
          sprintf(faseBuff, "fase%d", i);
          update("locstatussmf", faseBuff, 2, unity);
        }
      }

      for(int i = 1; i<16; i++)
      {
        pedestrianPhase[i] = (pedestrianInt>>i)&1;
        char faseBuff[7];
        sprintf(faseBuff, "fasep%d", i);
        update("locstatussmf", faseBuff, pedestrianPhase[i], unity);
      }
        
      update("locstatussmf", "tp_ciclo", cycleTimer, unity);
      update("locstatussmf", "tp_sync", syncTimer, unity);
      update("locstatussmf", "next_fase", nextFase, unity);
    }
  }
}

int main()
{
 while (1)
  getPhases("locstatussmf");
}
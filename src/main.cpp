#include <stdio.h>
#include "../inc/snmpGet.h"
#include <pqxx/pqxx>
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;
using namespace pqxx;
using namespace this_thread;
using namespace chrono;

int greenInt ;
int yellowInt;
int redInt   ;

int greenPhase[16];
int yellowPhase[16];
int redPhase[16];

bool pronto = 0;
int id;

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
  sprintf(buff, "UPDATE %s set %s = %d where ID=%d", __table.c_str(), __column.c_str(), value, id);

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
    if(c[3].as<int>())
    {
      id = c[0].as<int>();
      pronto = 1;

      char ipS[30];
      sprintf(ipS, "%s:%d", c[2].as<string>().c_str(), 161);

      greenInt  =  snmpGet(".1.3.6.1.4.1.1206.4.2.1.1.4.1.4.1", ipS);
      yellowInt =  snmpGet(".1.3.6.1.4.1.1206.4.2.1.1.4.1.3.1", ipS);
      redInt    =  snmpGet(".1.3.6.1.4.1.1206.4.2.1.1.4.1.2.1", ipS);

      for(int i = 0; i<15; i++)
        greenPhase[i] = (greenInt>>i)&1;
      for(int i = 0; i<15; i++)
        yellowPhase[i] = (yellowInt>>i)&1;
      for(int i = 0; i<15; i++)
        redPhase[i] = (redInt>>i)&1;
    
      update("PHASES", "RED", redInt, id);
      update("PHASES", "GREEN", greenInt, id);
      update("PHASES", "YELLOW", yellowInt, id);
    }
  }
}

int main()
{
 while (1)
  getPhases("SEMAPHORE");
  sleep_for(milliseconds(500));
}
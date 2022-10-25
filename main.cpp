#include <stdio.h>
#include "snmpGet.h"
#include <pqxx/pqxx>
#include <iostream>

using namespace std;
using namespace pqxx;

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

void get(const std::string &__table)
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
      char ipS[30];
      sprintf(ipS, "%s:%d", c[2].as<string>().c_str(), 161);

      int greenInt  =  snmpGet(".1.3.6.1.4.1.1206.4.2.1.1.4.1.4.1", ipS);
      int yellowInt =  snmpGet(".1.3.6.1.4.1.1206.4.2.1.1.4.1.3.1", ipS);
      int redInt    =  snmpGet(".1.3.6.1.4.1.1206.4.2.1.1.4.1.2.1", ipS);

      int greenPhase[16];
      int yellowPhase[16];
      int redPhase[16];
      for(int i = 0; i<15; i++)
        greenPhase[i] = (greenInt>>i)&1;
      for(int i = 0; i<15; i++)
        yellowPhase[i] = (yellowInt>>i)&1;
      for(int i = 0; i<15; i++)
        redPhase[i] = (redInt>>i)&1;

      char comando[50];

      sprintf(comando, "UPDATE PHASES set RED = %d where ID=%d", redInt, c[0].as<int>());
      N.exec(comando);  
      sprintf(comando, "UPDATE PHASES set GREEN = %d where ID=%d", greenInt, c[0].as<int>());
      N.exec(comando);
      sprintf(comando, "UPDATE PHASES set YELLOW = %d where ID=%d", yellowInt, c[0].as<int>());
      N.exec(comando);

    }
  }
  
}

int main()
{
  connection C("dbname = teste user = postgres password = docker \
    hostaddr = 127.0.0.1 port = 5432");

  if (C.is_open())
    printf("Banco de dados aberto com sucesso: %s\r\n", C.dbname());
  else {
    printf("Não foi possível abrir o banco de dados\r\n");
  }

  char * sql;

  update("PHASES", "GREEN", 20, 5);
  printf("Retorno : %s\r\n", sql);

  work W(C);

  sql = "CREATE TABLE SEMAPHORE("  \
  "ID INT PRIMARY KEY     NOT NULL," \
  "NAME           TEXT    NOT NULL," \
  "IP             TEXT," \
  "SEMEX          INT     NOT NULL DEFAULT 1);";

  W.exec(sql);
  W.commit();
  printf("Valores adicionados\r\n");

  C.disconnect(); 
  
}
#pragma region Apache License 2.0
/*
Nuclex Frame Fixer
Copyright (C) 2024 Markus Ewald / Nuclex Development Labs

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#pragma endregion // Apache License 2.0

// If the application is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_FRAMEFIXER_SOURCE 1

#include "FrameDatabase.h"
#include "../Model/DeinterlaceMode.h"
#include "../Model/FrameAction.h"

#include <QSqlQuery> // for QSqlQuery
#include <QSqlError> // for QSqlError
#include <QStringList> // for QStringList
#include <QVariant> // for QVariant
#include <QFile> // for QFile

#include <stdexcept> // for std::runtime_error

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Version number of the schema this application is implementing</summary>
  const std::size_t currentSchemaVersion = 1;

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Executes an SQL query and checks whether it was successful</summary>
  /// <param name="database">Database on which the SQL query will be run</param>
  /// <param name="sqlQuery">Query that will be executed</param>
  void executeSqlQueryAndCheck(
    const std::unique_ptr<QSqlDatabase> &database, const QString &sqlQuery
  ) {
    QSqlQuery query(*database.get());

    bool successfullyExecuted = query.exec(QString(sqlQuery));
    if(!successfullyExecuted) {
      std::string message(u8"Error executing query '");
      message.append(sqlQuery.toStdString());
      message.append(u8"': ");

      QSqlError lastError = query.lastError();
      if(lastError.type() != QSqlError::ErrorType::NoError) {
        message.append(lastError.text().toStdString());
      } else {
        message.append(u8"unknown QtSql error");
      }

      throw std::runtime_error(message);
    }
  }

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex::FrameFixer::Database {

  // ------------------------------------------------------------------------------------------- //

  FrameDatabase::FrameDatabase() :
    database() {}

  // ------------------------------------------------------------------------------------------- //

  FrameDatabase::~FrameDatabase() {
    CloseDatabase();
  }

  // ------------------------------------------------------------------------------------------- //

  void FrameDatabase::OpenOrCreateDatabase(const std::string &sqliteDatabasePath) {

    // Make sure Qt's SQLite driver is installed / built. This is mainly to provide
    // a clean error message and different exception type if the entire application
    // is compiled with Qt binaris that have no chance of ever working.
    bool sqliteDriverInstalled = QSqlDatabase::drivers().contains(u8"QSQLITE");
    if(!sqliteDriverInstalled) {
      throw std::logic_error(
        u8"Qt was built without its SQLite driver, unable to open databaase"
      );
    }

    // Close the database in case it was currently open (needed due to Qt's weird design
    // to unregister any prior database and allow for opening a new one).
    CloseDatabase();

    // Open a database. This also adds it so some secret, global list of databases
    // under the specified name.
    this->database = std::make_unique<QSqlDatabase>(
      QSqlDatabase::addDatabase("QSQLITE", QString(u8"frames"))
    );

    bool isExistingDatabase = QFile(QString::fromStdString(sqliteDatabasePath)).exists();

    // Now try to open or create the databse file.
    this->database->setDatabaseName(QString::fromStdString(sqliteDatabasePath));
    bool wasOpened = this->database->open();
    if(!wasOpened) {
      std::string message(u8"Failed to open frame database '");
      message.append(sqliteDatabasePath);
      message.append(u8"', ");
      
      QSqlError lastError = this->database->lastError();
      if(lastError.type() != QSqlError::ErrorType::NoError) {
        message.append(lastError.text().toStdString());
      } else {
        message.append(u8"unknown QtSql error");
      }

      throw std::runtime_error(message);
    }

    // Perform a schema upgrade if needed
    if(isExistingDatabase) {
      std::size_t schemaVersion = getSchemaVersion();
      if(schemaVersion < currentSchemaVersion) {
        upgradeSchema(schemaVersion);
      }
    } else { // New database implicitly has schema version 0
      upgradeSchema(0);
    }
    
  }

  // ------------------------------------------------------------------------------------------- //

  void FrameDatabase::CloseDatabase() {
    if(static_cast<bool>(this->database)) {
      this->database->close();
      this->database.reset();
      QSqlDatabase::removeDatabase(QString(u8"frames"));
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void FrameDatabase::upgradeSchema(std::size_t startingVersion) {
    switch(startingVersion) {
      case 0: { upgradeSchemaFrom0To1(); [[fallthrough]]; }
      case 1: { break; }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void FrameDatabase::upgradeSchemaFrom0To1() {
    using Nuclex::FrameFixer::FrameAction;

    // Basic application information to identify database
    {
      executeSqlQueryAndCheck(
        this->database, u8"CREATE TABLE application(name NVARCHAR(256) NOT NULL);"
      );
      executeSqlQueryAndCheck(this->database, u8"INSERT INTO application VALUES('FrameFixer');");
      executeSqlQueryAndCheck(
        this->database, u8"CREATE TABLE version(current INTEGER NOT NULL);"
      );
      executeSqlQueryAndCheck(this->database, u8"INSERT INTO version VALUES(1);");
    }

    // Deinterlace modes
    {
      executeSqlQueryAndCheck(
        this->database,
        u8"CREATE TABLE deinterlaceModes("
        u8"  id INTEGER PRIMARY KEY,"
        u8"  description NVARCHAR(64) NOT NULL"
        u8");"
      );

      QSqlQuery query(*this->database.get());
      bool successfullyPrepared = query.prepare(
        u8"INSERT INTO deinterlaceModes "
        u8"VALUES"
        u8"  (:progressiveEnumValue, 'Progressive'),"
        u8"  (:topFieldFirstEnumValue, 'TopFieldFirst'),"
        u8"  (:bottomFieldFirstEnumValue, 'BottomFieldFirst'),"
        u8"  (:topFieldOnlyEnumValue, 'TopFieldOnly'),"
        u8"  (:bottomFieldOnlyEnumValue, 'BottomFieldOnly')"
        u8";"
      );
      if(!successfullyPrepared) {
        std::string message(u8"Error preparing SQL statement to record deinterlace modes: ");

        QSqlError lastError = query.lastError();
        if(lastError.type() != QSqlError::ErrorType::NoError) {
          message.append(lastError.text().toStdString());
        } else {
          message.append(u8"unknown QtSql error");
        }

        throw std::runtime_error(message);
      }
      query.bindValue(u8":progressiveEnumValue", static_cast<int>(FrameAction::Progressive));
      query.bindValue(u8":topFieldFirstEnumValue", static_cast<int>(FrameAction::TopFieldFirst));
      query.bindValue(u8":bottomFieldFirstEnumValue", static_cast<int>(FrameAction::BottomFieldFirst));
      query.bindValue(u8":topFieldOnlyEnumValue", static_cast<int>(FrameAction::TopFieldOnly));
      query.bindValue(u8":bottomFieldOnlyEnumValue", static_cast<int>(FrameAction::BottomFieldOnly));

      // Query is set up and all parameters are bound, now we can execute it
      bool successfullyExecuted = query.exec();
      if(!successfullyExecuted) {
        std::string message(u8"Error executing SQL statement to record frame types: ");

        QSqlError lastError = query.lastError();
        if(lastError.type() != QSqlError::ErrorType::NoError) {
          message.append(lastError.text().toStdString());
        } else {
          message.append(u8"unknown QtSql error");
        }

        throw std::runtime_error(message);
      }
    }

    {
      executeSqlQueryAndCheck(
        this->database,
        u8"CREATE TABLE frameActions("
        u8"  id INTEGER PRIMARY KEY,"
        u8"  description NVARCHAR(64) NOT NULL"
        u8");"
      );

      QSqlQuery query(*this->database.get());
      bool successfullyPrepared = query.prepare(
        u8"INSERT INTO frameActions "
        u8"VALUES"
        u8"  (:discardEnumValue, 'Discard'),"
        u8"  (:keepEnumValue, 'Keep'),"
        u8"  (:replaceEnumValue, 'Replace'),"
        u8"  (:duplicateEnumValue, 'Duplicate'),"
        u8"  (:triplicateEnumValue, 'Triplicate'),"
        u8"  (:deblendEnumValue, 'Deblend')"
        u8";"
      );
      if(!successfullyPrepared) {
        std::string message(u8"Error preparing SQL statement to record deinterlace modes: ");

        QSqlError lastError = query.lastError();
        if(lastError.type() != QSqlError::ErrorType::NoError) {
          message.append(lastError.text().toStdString());
        } else {
          message.append(u8"unknown QtSql error");
        }

        throw std::runtime_error(message);
      }
      query.bindValue(u8":discardEnumValue", static_cast<int>(FrameAction::Discard));
      query.bindValue(u8":keepEnumValue", static_cast<int>(FrameAction::Unknown));
      query.bindValue(u8":replaceEnumValue", static_cast<int>(FrameAction::Replace));
      query.bindValue(u8":duplicateEnumValue", static_cast<int>(FrameAction::Duplicate));
      query.bindValue(u8":triplicateEnumValue", static_cast<int>(FrameAction::Triplicate));
      query.bindValue(u8":deblendEnumValue", static_cast<int>(FrameAction::Deblend));

      // Query is set up and all parameters are bound, now we can execute it
      bool successfullyExecuted = query.exec();
      if(!successfullyExecuted) {
        std::string message(u8"Error executing SQL statement to record frame types: ");

        QSqlError lastError = query.lastError();
        if(lastError.type() != QSqlError::ErrorType::NoError) {
          message.append(lastError.text().toStdString());
        } else {
          message.append(u8"unknown QtSql error");
        }

        throw std::runtime_error(message);
      }
    }

    executeSqlQueryAndCheck(
      this->database,
      u8"CREATE TABLE frames("
      u8"  id INTEGER PRIMARY KEY,"
      u8"  number INTEGER NOT NULL,"
      u8"  filename NVARCHAR(256) NOT NULL,"
      u8"  mode INTEGER,"
      u8"  action INTEGER,"
      u8"  leftIndex INTEGER,"
      u8"  rightIndex INTEGER,"
      u8"  combedness REAL,"
      u8"  similarityToPrevious REAL,"
      u8"  FOREIGN KEY(mode) REFERENCES deinterlaceModes(id)"
      u8"  FOREIGN KEY(action) REFERENCES frameActions(id)"
      u8");"
    );

    executeSqlQueryAndCheck(
      this->database,
      u8"CREATE INDEX framesByNumber ON frames("
	    u8"  number"
      u8")"
    );
    // PRAGMA table_info(frames) = (0, 'uid', 'INTEGER', 0, 'User ID');
  }

  // ------------------------------------------------------------------------------------------- //

  std::size_t FrameDatabase::getSchemaVersion() const {
    if(!static_cast<bool>(this->database)) {
      throw std::runtime_error(u8"Cannot determine schema version, no database opened");
    }

    // This method is only called when an existing database was opened. If it doesn't
    // have the version table, it's not 
    if(!this->database->tables().contains("version")) {
      throw std::runtime_error(
        u8"SQLite frame database has no version table. "
        u8"This is probably not a Frame Fixer frame database."
      );
    }

    // Run a query to figure out the current database version
    QSqlQuery query(*this->database.get());
    bool successfullyExecuted = query.exec(QString(u8"SELECT current FROM version"));
    if(!successfullyExecuted) {
      std::string message(u8"Error querying schema version from database: ");

      QSqlError lastError = query.lastError();
      if(lastError.type() != QSqlError::ErrorType::NoError) {
        message.append(lastError.text().toStdString());
      } else {
        message.append(u8"unknown QtSql error");
      }

      throw std::runtime_error(message);
    }

    // Read the one and only row in this table to obtain the schema version
    bool versionRowFound = query.next();
    if(!versionRowFound) {
      throw std::runtime_error(
        u8"Frame database has no rows in its version table. "
        u8"This is probably not a Frame Fixer frame database."
      );
    }

    return query.value(0).toInt();
  }

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Database

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

#ifndef NUCLEX_FRAMEFIXER_DATABASE_FRAMEDATABASE_H
#define NUCLEX_FRAMEFIXER_DATABASE_FRAMEDATABASE_H

#include "Nuclex/FrameFixer/Config.h"

//#include "./Frame.h" // for Frame
//#include "./AdoptedProcess.h" // for AdoptedProcess

#include <memory> // for std::unique_ptr
#include <vector> // for std::vector

#include <QSqlDatabase> // for QSqlDatabase

namespace Nuclex::FrameFixer::Database {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Manages the SQLite database in which frame metadata is stored</summary>
  class FrameDatabase {

    /// <summary>Initializes the wrapper for frame databases</summary>
    public: FrameDatabase();

    /// <summary>Closes the frame database and frees all memory used</summary>
    public: ~FrameDatabase();

    /// <summary>Opens the specified frame database</summary>
    /// <param name="sqliteDatabasePath">
    ///   Path fo the frame database that will be opened
    /// </param>
    public: void OpenOrCreateDatabase(const std::string &sqliteDatabasePath);

    /// <summary>Closes the current frame database</summary>
    public: void CloseDatabase();

    // ----------------------------------------------------------------------------------------- //
#if 0
    /// <summary>Lists all processes that have been adopted by CriuGui</summary>
    /// <returns>A list of all the processes adopted into CriuGui</returns>
    public: std::vector<AdoptedProcess> ListAdoptedProcesses() const;

    /// <summary>Adds a process into the frame database</summary>
    /// <param name="newProcess">Metadata for the process that will be added</param>
    /// <returns>The unique id of the process in the database</returns>
    public: std::size_t AddAdoptedProcess(const AdoptedProcess &newProcess);

    /// <summary>Removes an adopted process from the database</summary>
    /// <param name="adoptedProcessId">Id of the adopted process that will be removed</param>
    /// <returns>True if the adopted process existed and was removed</returns>
    public: bool RemoveAdoptedProcess(std::size_t adoptedProcessId);

    // ----------------------------------------------------------------------------------------- //

    /// <summary>Lists all frames that have been recorded for a process</summary>
    /// <param name="adoptedProcessID">Process for which frames will be listed</param>
    /// <returns>A list of all the processes adopted into CriuGui</returns>
    public: std::vector<Frame> ListFrames(std::size_t adoptedProcessId) const;

    /// <summary>Adds a frame into the frame database</summary>
    /// <param name="newFrame">Metadata for the frame that will be added</param>
    /// <returns>The unique id of the frame in the database</returns>
    public: std::size_t AddFrame(const Frame &newFrame);

    /// <summary>Removes a frame from the database</summary>
    /// <param name="frameId">Id of the frame that will be removed</param>
    /// <returns>True if the frame existed and was removed</returns>
    public: bool RemoveFrame(std::size_t frameId);
#endif
    // ----------------------------------------------------------------------------------------- //

    /// <summary>Determines the schema version of the opened database</summary>
    /// <returns>The database schema version</returns>
    private: std::size_t getSchemaVersion() const;

    /// <summary>Upgrades the database schema from the specified version</summary>
    /// <param name="startingVersion">Schema version data database is at currently</param>
    private: void upgradeSchema(std::size_t startingVersion);

    /// <summary>Updates the database schema from version 0 to version 1</summary>
    private: void upgradeSchemaFrom0To1();

    /// <summary>Database in which the frame informations are stored</summary>
    /// <remarks>
    ///   The entire QSql design is... weird like this. Copies around entire classes which
    ///   only define vanilla copy constructors. Factory methods instead of constructors
    ///   forcing additional copies on top of it. Queries are magically picking a datbase
    ///   to run on from secret global variables. The entire design is quite bonkers...
    /// </remarks>
    private: std::unique_ptr<QSqlDatabase> database;

  };

  // ------------------------------------------------------------------------------------------- //

} // namespace Nuclex::FrameFixer::Database

#endif // NUCLEX_FRAMEFIXER_DATABASE_FRAMEDATABASE_H

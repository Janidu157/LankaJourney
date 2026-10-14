CREATE DATABASE IF NOT EXISTS LankaJourney;

-- DROP DATABASE lankaJourney;

USE LankaJourney;

CREATE TABLE Location (
	Location_ID VARCHAR(10) PRIMARY KEY,
    Latitude DECIMAL(9,6),
    Longitude DECIMAL(9,6),
    Town VARCHAR(50),
    District VARCHAR(50),
    Province_code VARCHAR(10)
);

-- DROP TABLE Location

CREATE TABLE Accommodation (
	Accom_ID VARCHAR(50) PRIMARY KEY,
    Accom_Name VARCHAR(50) ,
    Accom_Type VARCHAR(20),
    Accom_Price FLOAT,
    Location_ID VARCHAR(10),
    Rating FLOAT,
    Food_Avialability BOOLEAN,
    Website VARCHAR(200),
    FOREIGN KEY (Location_ID) REFERENCES Location(Location_ID)
);

-- DROP TABLE Accommodation;

CREATE TABLE Tourist_Places (
	Place_ID VARCHAR(10) PRIMARY KEY,
    Place_Name VARCHAR(50),
    Location_ID VARCHAR(10),
    Place_Type VARCHAR(20),
    Visiting_Experience VARCHAR(20),
    Climate VARCHAR(10),
    Accessibility VARCHAR(10),
    Diff_Level VARCHAR(10),
    FOREIGN KEY (Location_ID) REFERENCES Location(Location_ID)
);

-- DROP TABLE Tourist_Places;


CREATE TABLE Vehicle_Agency(
	Vehicle_Agent_ID VARCHAR(10) PRIMARY KEY,
	Agency_Name VARCHAR (50),
	Location_ID VARCHAR(10),
	Contact VARCHAR (50),
	FOREIGN KEY(Location_ID) REFERENCES Location(Location_ID)
);

-- DROP TABLE Vehicle_Agency;

CREATE TABLE Vehicle_Rent (
	Option_ID VARCHAR(10) PRIMARY KEY,
    Vehicle_Agent_ID VARCHAR(10),
    Vehicle_Category VARCHAR(10),
    Number_of_Passengers INT,
    Luggage_Amount INT,
    Need_Driver BOOLEAN, 
    FOREIGN KEY (Vehicle_Agent_ID) REFERENCES Vehicle_Agency(Vehicle_Agent_ID)
);

-- DROP TABLE Vehicle_Rent;


CREATE TABLE Restaurant(
	Restaurant_ID VARCHAR(10) PRIMARY KEY,
	Restaurant_Name VARCHAR (50) NOT NULL,
	Location_ID VARCHAR(10),
	Rating DECIMAL(2,1),
	Contact VARCHAR(20),
	FOREIGN KEY (Location_ID) REFERENCES Location(Location_ID)
);

-- DROP TABLE Restaurant;

-- SELECT * FROM Location;
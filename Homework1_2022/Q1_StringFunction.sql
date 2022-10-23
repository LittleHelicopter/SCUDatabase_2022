/*
Q1 StringFunction:
Get all unique ShipNames from the Order table that contain a hyphen '-'.
Details: In addition, get all the characters preceding the (first) hyphen. Return ship
names alphabetically. Your first row should look like Bottom-Dollar Markets|Bottom
*/
SELECT DISTINCT shipName || "|" || 
                SUBSTRING(shipName,1,instr(shipName,'-')-1)     -- '||' Connector
FROM 'Order' 
WHERE shipName LIKE "%-%"       -- '%' Any string containing zero or more characters
ORDER BY shipName;

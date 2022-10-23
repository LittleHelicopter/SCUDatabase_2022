/*
Q8 YoungBlood:
Find the youngest employee serving each Region. If a Region is not served by an
employee, ignore it.
Details: Print the Region Description, First Name, Last Name, and Birth Date.
Order by Region Id.
Your first row should look like Eastern|Steven|Buchanan|1987-03-04
*/

SELECT RegionDescription || "|" || FirstName 
       || "|" || LastName || "|" || MAX(BirthDate)
FROM(SELECT * FROM Employee
     LEFT OUTER JOIN EmployeeTerritory ON Employee.Id=EmployeeTerritory.employeeId
     LEFT OUTER JOIN Territory ON EmployeeTerritory.TerritoryId=Territory.Id
     LEFT OUTER JOIN Region ON Territory.RegionId=Region.Id
     )
GROUP BY RegionId
ORDER BY RegionId;

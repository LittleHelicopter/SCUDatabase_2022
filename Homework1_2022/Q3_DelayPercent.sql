/*
Q3 DELAYPERCENT:
For each Shipper, find the percentage of orders which are late.
Details: An order is considered late if ShippedDate > RequiredDate. Print the
following format, order by descending precentage, rounded to the nearest
hundredths, like United Package|23.44
*/

SELECT CompanyName || "|" || 
       printf("%.2f", COUNT(IIF(ShippedDate>RequiredDate, 1, null))/ROUND(COUNT('Order'.Id))*100)
FROM "Order" LEFT OUTER JOIN Shipper
WHERE "Order".shipVia = Shipper.Id 
GROUP BY Shipper.Id
ORDER BY (COUNT(IIF(ShippedDate>RequiredDate, 1, null))/ROUND(COUNT('Order'.Id))*100) DESC;

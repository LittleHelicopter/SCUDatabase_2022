/*
Q5 Discontinued:
For each of the 8 discontinued products in the database, which customer made the
first ever order for the product? Output the
customer's CompanyName and ContactName
Details: Print the following format, order by ProductName alphabetically: Alice
Mutton|Consolidated Holdings|Elizabeth Brown
*/
SELECT ProductName || "|" || CompanyName || "|" || ContactName
FROM (SELECT *, MIN(OrderDate)
      FROM "Order"
      LEFT OUTER JOIN OrderDetail ON "Order".Id=OrderId 
      LEFT OUTER JOIN Customer ON CustomerId=Customer.Id 
      LEFT OUTER JOIN Product ON ProductId=Product.Id 
      WHERE Discontinued = 1
      GROUP BY ProductId 
      ) 
GROUP BY ProductId 
ORDER BY ProductName;

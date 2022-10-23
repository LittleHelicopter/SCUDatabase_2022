/*
Q7 TotalCostQuartiles:
For each Customer, get the CompanyName, CustomerId, and "total expenditures".
Output the bottom quartile of Customers, as measured by total expenditures.
Details: Calculate expenditure using UnitPrice and Quantity (ignore Discount).
Compute the quartiles for each company's total expenditures using NTILE. The
bottom quartile is the 1st quartile, order them by increasing expenditure.
Make sure your output is formatted as follows (round expenditure to nearest
hundredths): Bon app|BONAP|4485708.49
Note: There are orders for CustomerIds that don't appear in the Customer table.
You should still consider these "Customers" and output them. If
the CompanyName is missing, override the NULL to 'MISSING_NAME' using IFNULL.
*/

SELECT IFNULL(CompanyName, "MISSING_NAME") || "|"
       || CustomerId || "|" || ROUND(TotalExpenditures,2)
FROM(SELECT CompanyName, CustomerId, TotalExpenditures, 
            NTILE(4) OVER(ORDER BY CAST(TotalExpenditures AS float)) AS quartile 
     FROM(SELECT CustomerId, SUM((UnitPrice*Quantity)) AS TotalExpenditures 
          FROM "Order" 
          LEFT OUTER JOIN OrderDetail ON "Order".Id = OrderDetail.OrderId 
          GROUP BY "Order".CustomerId)
     LEFT OUTER JOIN Customer ON CustomerId = Customer.Id)
WHERE quartile=1
ORDER BY CAST(TotalExpenditures AS float);

/*
Q9 Chrismas:
Concatenate the ProductNames ordered by the Company 'Queen
Cozinha' on 2014-12-25.
Details: Order the products by Id (ascending). Print a single string
containing all the dup names separated by commas like Mishi Kobe, 
Niku, NuNuCa Nuß-Nougat-Creme...
Hint: You might find Recursive CTEs useful.
*/
SELECT GROUP_CONCAT(ProductName, ", ")       --Print the value of the 'ProductName' field on one line, separated by a semicolon
FROM "Order"
     LEFT OUTER JOIN OrderDetail ON "Order".Id = OrderDetail.OrderId 
     LEFT OUTER JOIN Product ON OrderDetail.ProductId = Product.Id 
     LEFT OUTER JOIN Customer ON "Order".CustomerId = Customer.Id 
WHERE Customer.CompanyName = 'Queen Cozinha' 
      AND "Order".OrderDate LIKE "2014-12-25 __:__:__";      --_ (underscore) stands for any single character

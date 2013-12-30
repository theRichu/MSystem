#
# ShinyMongo App
# a simple R based MongoDB - Viewer
# 
# Markus Schmidberger, markus.schmidberger@comsysto.com
# June, 2013

library(shiny)
library(rmongodb)
library(rjson)
#RJSONJO gets problems with big JSON objects )-:

library("plyr")

last <- function(x) { return( x[length(x)] ) }

bsonToList <-function(key,v){
        val <- NULL
        for ( mykey in key){
                if (is.null(mongo.bson.value(v,mykey))){
                        val <- cbind(val,NA)
                } else {
                        if(mykey=="_id"){
                                val <- cbind(val,as.character(mongo.bson.value(v,mykey)))
                        }else{
                                val <- cbind(val,mongo.bson.value(v,mykey))
                        }
                }
        }
        return(val)
}

cursor2DataFrame <-function(cursor){
        d.key          <- NULL
        c       <- NULL
        DF                 <- NULL
        i                 <- 1

        while (mongo.cursor.next(cursor)) {        
                if(i==1){
                        iter  <- mongo.bson.iterator.create(mongo.cursor.value(cursor))
                        while (mongo.bson.iterator.next(iter)){
                                if(mongo.bson.iterator.type(iter)!=3){
                                                d.key <- cbind(d.key,mongo.bson.iterator.key(iter)) 
                                }else{
                                        iter2  <- mongo.bson.iterator.create(iter)
                                        rootkey<- mongo.bson.iterator.key(iter)
                                        while (mongo.bson.iterator.next(iter2)){
                                                d.key <-  cbind(d.key,paste(rootkey,mongo.bson.iterator.key(iter2),sep="."))
                                        }
                                }
                        }
                }
                i           <- i+1
                v           <- mongo.cursor.value(cursor)
                d.val <- bsonToList(d.key,v)
                c           <- rbind(c,c(d.val))
        }

        DF <- as.data.frame(c)
        colnames(DF)<-d.key
        return(DF)
}


# parameter to set the maximum queyering and displaying lentgth
limit <- 100L

shinyServer(function(input, output) {

  # create mongo connection
  connection <- reactive({
    mongo <- mongo.create(input$host, username=input$username, password=input$password)
  })
  
  ####################
  # sidebar rendering
  ####################
  
  # render database input field
  output$dbs <- renderUI({
    mongo <- connection()
    if (mongo.is.connected(mongo)) {
      dbs <- mongo.get.databases(mongo)
      selectInput("db_input", "Database", dbs)
    }
  })
  
  # render collection input field
  output$collections <- renderUI({
    mongo <- connection()
    if (mongo.is.connected(mongo)) {
      if( !is.null(input$db_input) ){
        collections <- mongo.get.database.collections(mongo, input$db_input)
        selectInput("collections_input", "Collections", c("-",collections))
      }
    }
  })
  
  # render query input field
  output$query <- renderUI({  
    mongo <- connection()
    if (mongo.is.connected(mongo)) {
      if( !is.null(input$collections_input)){
        if( !is.null(input$db_input) && input$collections_input!="-" ){
          textInput("query", "JSON Query - experimental:", "")
        }
      }
    }
  })


  output$column_list = renderUI({
       mongo <- connection()
    if (mongo.is.connected(mongo)) {
    
      if( !is.null(input$query) ){
        if( input$query !="" ){
          Rquery <- fromJSON(input$query)
          query <- mongo.bson.from.list(Rquery)
        } else {
          buf <- mongo.bson.buffer.create()
          query <- mongo.bson.from.buffer(buf)
        }
      }
      
      if( !is.null(input$collections_input) && !is.null(input$query) ){
        count <- mongo.count(mongo, input$collections_input, query)
        cursor <- mongo.find(mongo, input$collections_input, query, limit=limit)

        df= cursor2DataFrame(cursor)
 
       mongo.cursor.destroy(cursor)

           checkboxGroupInput('show_vars', 'Columns to show:', names(df),
                       selected = names(df))
      }
    }
  })



  
  
  ####################
  # output / main window rendering
  ####################
  
  # display text for connection information / error
  output$connection <- renderText({
    mongo <- connection()
    if (mongo.is.connected(mongo)) {
      str <- mongo.get.primary(mongo)
      paste("Connected to ", str , sep="")
    } else {
      # ToDo: more detailed errors
      paste("Unable to connect.  Error code:", mongo.get.err(mongo))
    }
  })
  
  # display collection data as JSON output
  output$view <- renderText({
    mongo <- connection()
    if (mongo.is.connected(mongo)) {
    
      if( !is.null(input$query) ){
        if( input$query !="" ){
          Rquery <- fromJSON(input$query)
          query <- mongo.bson.from.list(Rquery)
        } else {
          buf <- mongo.bson.buffer.create()
          query <- mongo.bson.from.buffer(buf)
        }
      }
        
      if( !is.null(input$collections_input) && !is.null(input$query) ){
        cursor <- mongo.find(mongo, input$collections_input, query, limit=limit)
        res_list <- list()
        tmp_all <- NULL
        i <- 1
       while (mongo.cursor.next(cursor)){
         tmp <- mongo.cursor.value(cursor)
         res_list[[i]] <- mongo.bson.to.list(tmp)
         i <- i+1
       }

       mongo.cursor.destroy(cursor)
        json <- toJSON(res_list)
        json <- gsub("\\},\\{", "},<br><br>{", json)
       return( json )
      }
    }
  })


  output$mytable1 = renderDataTable({
       mongo <- connection()
    if (mongo.is.connected(mongo)) {
    
      if( !is.null(input$query) ){
        if( input$query !="" ){
          Rquery <- fromJSON(input$query)
          query <- mongo.bson.from.list(Rquery)
        } else {
          buf <- mongo.bson.buffer.create()
          query <- mongo.bson.from.buffer(buf)
        }
      }
      
      if( !is.null(input$collections_input) && !is.null(input$query) ){
        count <- mongo.count(mongo, input$collections_input, query)
        cursor <- mongo.find(mongo, input$collections_input, query, limit=limit)

        df= cursor2DataFrame(cursor)
 
       mongo.cursor.destroy(cursor)
       library(ggplot2)
      # df
       df[, input$show_vars, drop = FALSE]
      }
    }
  })
















        
  # display Headline for collection data output
  output$view_head <- renderText({
    mongo <- connection()
    if (mongo.is.connected(mongo)) {
      if( !is.null(input$collections_input) && input$collections_input != "-" ){
        count <- mongo.count(mongo, input$collections_input)
        if (count < limit)
          limit <- count
        paste("Documents (", limit, " out of ",count,")", sep="")
      }
    }
  })
  
  # display table with collection overview
  output$view_collections <- renderTable({
    mongo <- connection()
    if( mongo.is.connected(mongo) ) {
      if( !is.null(input$db_input) ){
        coll <- mongo.get.database.collections(mongo, input$db_input)
        
        res <- NULL
        for(i in coll){
          val <- mongo.count(mongo, i)
          tmp <- cbind(i,val)
          res <- rbind(res, tmp)
        }
        if( !is.null(res) )
          colnames(res) <- c("collection", "# documents")
        
        return(res)
      }
    } 
  })

})

#
# ShinyMongo App
# a simple R based MongoDB - Viewer
# 
# Markus Schmidberger, markus.schmidberger@comsysto.com
# June, 2013

library(shiny)
library(ggplot2)  # for the diamonds dataset

# Define UI for miles per gallon application
shinyUI(pageWithSidebar(
  
  # Application title
  headerPanel("M System Prototype"),
  
  # Sidebar
  sidebarPanel(
    
    textInput("host", "Host:", "localhost"),
    textInput("username", "Username:", ""),
    textInput("password", "Password:", ""),
    
    uiOutput("dbs"),
  
    uiOutput("collections"),
    
    uiOutput("query"),

    uiOutput("column_list"),

    br(),br(),
    helpText("Development: TaeYeong Kim therichu@gmail.com"),
    helpText("more at https://github.com/therichu/MSystem")
    ),
  
  # main window
  mainPanel(
    
    textOutput("connection"),
    
    tabsetPanel(
      tabPanel('man', dataTableOutput("mytable1"))
    ),

    conditionalPanel(
      condition = "input.collections_input == '-'",
      h4("Collections overview:"),
      tableOutput("view_collections")
    ),
    
    conditionalPanel(
      condition = "input.collections_input != '-'",
      h4(textOutput("view_head")),
      tableOutput("view")
    )
    
  )

))

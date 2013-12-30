var mongoose = require('mongoose');

var MeasureSchema = new mongoose.Schema({
  type: String,
  data: String
});

module.exports = MeasureSchema;

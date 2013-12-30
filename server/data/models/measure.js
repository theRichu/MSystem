var mongoose = require('mongoose');
var MeasureSchema = require('../schemas/measure');

var Measure = mongoose.model('Measure', MeasureSchema);

module.exports = Measure;


/*
 * check module
 * 
 */

require('date-utils');

var _mongojs = require("mongojs");

var _dbProfessorChecker = _mongojs('professorChecker');
var _collectionStateLog = _dbProfessorChecker.collection('statelog');

exports.checkIn = function (req, res) {

	var state_string = "상태 없음";

	// 0 = sit up.
	// 1 = sit down
	var current_date = new Date();

	var saving_data = {

		"state": req.body.state,
		"time": current_date
	};

	res.send(_collectionStateLog.save(saving_data, function () {

		console.log("=============================");
		console.log("Saving complete");
		console.log("time: " + saving_data.time);
		console.log("=============================");
		console.log(" ");
	}));
};

exports.checkOut = function (resultCallback) {

	_collectionStateLog.find().sort({"time":-1}).limit(1, function (err, doc) {

		if ( (err) || (undefined == doc[0]) || (undefined == doc[0].state) ) {

			return resultCallback(
			{
				state: -1,
				time: new Date()
			})
		}

		resultCallback(doc[0]);
	});
}

exports.stats = function (renderStatsPageCallback) {

	_collectionStateLog.find().sort({"time":-1}).limit(20, function (err, doc) {

		if ( (err) || (undefined == doc[0]) || (undefined == doc[0].state) ) {

			return renderStatsPageCallback([{
				state: -1,
				time: new Date()
			}]);
		}

		return renderStatsPageCallback(doc);
	});
}
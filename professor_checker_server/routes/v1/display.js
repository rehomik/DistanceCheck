
/**
 * display module
 */

var check_v1 = require("./check");

exports.index = function (req, res) {

    res.end();
};

exports.displayState = function (req, res){

    check_v1.checkOut(function (docs) {

        var current_state = docs.state;
        var current_state_string = "...몰라요ㅜㅜ";

        if (1 == current_state) {

            current_state_string = "계십니다.";
        }
        else if (0 == current_state) {

            current_state_string = "안계십니다.";
        }

        var current_state_time = 
        "변경 시간: " + docs.time.toFormat('YYYY년 MM월 DD일 HH24:MI:SS');

        res.render(
            "v1/display_display",
            {
                title_string: "교수님 자리 확인 시스템",
                state_string: current_state_string,
                state_time: current_state_time
            })
    });
};

exports.displayStats = function(req, res) {

    check_v1.stats(function (stats_array) {

        // var d = dt.toFormat('YYYY년 MM월 DD일 HH24:MI:SS');
        var row_datas = [];

        for (var i = 0; i < stats_array.length; ++i) {

            var stats_data = stats_array[i];
            var stats_date = stats_data.time;

            stats_data.time = stats_date.toFormat("YYYY년 MM월 DD일 HH24:MI:SS");

            stats_array[i] = stats_data;
        }

        res.render(
            "v1/display_stats",
            {
                "row_datas": stats_array
            }
        );
    });
};
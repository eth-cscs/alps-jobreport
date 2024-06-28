#ifndef JOBREPORT_STATUS_HPP
#define JOBREPORT_STATUS_HPP
// Define struct for successful return or error message
enum Status {
    Success = 0,
    Error = 1,
    Help = 2,
    MissingValue = 3,
    InvalidValue = 4,
    MissingNonArguments = 5,
    MissingArgument = 6
};
#endif
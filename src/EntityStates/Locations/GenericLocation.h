/// Emil Hedemalm
/// 2013-02-08

#ifndef GENERIC_LOCATION
#define GENERIC_LOCATION


/// Subclass of location, the generic location can be used to socialize in ^^
struct GenericLocation : Location {
public:
	/// Default constructor
	GenericLocation() : Location(LocType::GENERIC){
		socialFactor = 1.0f;
	};
	/// Factor at which socializing is enhanced or penalized in this area.
	float socialFactor;


};
#endif

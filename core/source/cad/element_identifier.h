
#if !defined(ELEMENT_IDENTIFIER_H)
#define ELEMENT_IDENTIFIER_H

#include <map>

#include <Quantity_Color.hxx>

#include "api/cmiss_field_cad.h"
#include "cad/cad_element.h"

struct Cmiss_cad_identifier
{
	Cmiss_field_cad_topology_id cad_topology;
	Cad_primitive_identifier identifier;

	Cmiss_cad_identifier(Cmiss_field_cad_topology_id cad_topology, Cad_primitive_identifier identifier);
	~Cmiss_cad_identifier();

	Cmiss_cad_identifier(const Cmiss_cad_identifier& cad_identifier);
	Cmiss_cad_identifier& operator=(const Cmiss_cad_identifier& source);
	bool operator==(const Cmiss_cad_identifier& other) const;
	bool operator!=(const Cmiss_cad_identifier& other) const;
};

struct Cad_topology_primitive_identifier
{
private:
	int surface_index;
	int curve_index;
	int point_index;

public:
	Cad_topology_primitive_identifier(int surface_index = -1, int curve_index = -1, int point_index = -1)
		: surface_index(surface_index)
		, curve_index(curve_index)
		, point_index(point_index)
	{
	}

	~Cad_topology_primitive_identifier()
	{
	}

	inline int getSurfaceIndex() const {return surface_index;}
	inline int getCurveIndex() const {return curve_index;}
	inline int getPointIndex() const {return point_index;}
};

class Cad_topology_primitive_identifier_compare
{
public:
	bool operator() (const Cad_topology_primitive_identifier &id1, const Cad_topology_primitive_identifier &id2) const
	{
		if (id1.getSurfaceIndex() < id2.getSurfaceIndex())
			return true;
		else if (id1.getSurfaceIndex() == id2.getSurfaceIndex())
		{
			if (id1.getCurveIndex() < id2.getCurveIndex())
				return true;
			else if (id1.getCurveIndex() == id2.getCurveIndex())
			{
				if (id1.getPointIndex() < id2.getPointIndex())
					return true;
			}
		}
		return false;
	}
};



struct Cmiss_cad_colour
{
	enum Cmiss_cad_colour_type {
		CMISS_CAD_COLOUR_NOT_DEFINED = -1,
		CMISS_CAD_COLOUR_GENERIC = 0,
		CMISS_CAD_COLOUR_SURFACE = 1,
		CMISS_CAD_COLOUR_CURVE = 2
	};

private:
	Cmiss_cad_colour_type colour_type;
	Quantity_Color colour;

public:
	Cmiss_cad_colour(Cmiss_cad_colour_type colour_type = CMISS_CAD_COLOUR_NOT_DEFINED
		, Quantity_Color colour = Quantity_NOC_WHITE)
		: colour_type(colour_type)
		, colour(colour)
	{
	}

	Cmiss_cad_colour_type getColourType() const {return colour_type;}
	Quantity_Color getColour() const {return colour;}
};

typedef std::map<Cad_topology_primitive_identifier,Cmiss_cad_colour, Cad_topology_primitive_identifier_compare> Cad_colour_map;
typedef std::map<Cad_topology_primitive_identifier,Cmiss_cad_colour, Cad_topology_primitive_identifier_compare>::iterator Cad_colour_map_iterator;
typedef std::map<Cad_topology_primitive_identifier,Cmiss_cad_colour, Cad_topology_primitive_identifier_compare>::const_iterator Cad_colour_map_const_iterator;


#endif

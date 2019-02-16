#pragma once

class IStackable
{
private:
	//int m_type;
public:
	//IStackable( int type ):m_type( type ){}
	//virtual int getType() const { return m_type; } 

	//virtual bool isType( const IStackable& obj ) const = 0 ;
    virtual IStackable * copy( void ) const = 0;
    virtual ~IStackable( ){}
};
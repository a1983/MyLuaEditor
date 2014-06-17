#ifndef VARIABLEINFO_H
#define VARIABLEINFO_H

#include <QMap>
#include <QList>
#include <QVariant>

class VariableInfo
{
public:
	enum Type {
		Nil,
		Boolean,
		Number,
		String,
		Table,
		Function,
		UserData,
		LightUserData
	};

	enum Properties {
		TextProperty
	};

public:
	VariableInfo();
	~VariableInfo();

public:
	// Properties
	void SetText( const QString& text );
	const QString Text() const;

public:
	// Children management
	VariableInfo* Parent();
	VariableInfo* Child( int index );

	void AppendChild( VariableInfo* child );

	int SiblingPos() const;
	int ChildrenCount() const;

private:
	VariableInfo* _parent;
	QList< VariableInfo* > _children;
	QMap< VariableInfo*, int > _indexByChild;

	QMap< Properties, QVariant > _properties;
};

#endif // VARIABLEINFO_H

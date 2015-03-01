#include "RtRoot.h"
#include "RtVector.h"
#include "RtSession.h"

#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaEnum>

#include <QDateTime>

#include <H5Cpp.h>

using namespace H5;

void writeObject(H5File* file, RtObject* obj);
void writeFileAttribute(H5File* h5obj, const char* name, const char* value);

QString RtRoot::h5write(const QString& filename, const QString& comment)
{
	QString S;
	H5File *file = 0;
    // Try block to detect exceptions raised by any of the calls inside it
    try
    {
        /*
         * Turn off the auto-printing when failure occurs so that we can
         * handle the errors appropriately
         */
        Exception::dontPrint();

        /*
         * Create the named file, truncating the existing one if any,
         * using default create and access property lists.
         */
		file = new H5File( filename.toLatin1(), H5F_ACC_TRUNC );

		writeFileAttribute(file, "Timestamp", QDateTime::currentDateTime().toString(Qt::ISODate).toLatin1().constData());
		writeFileAttribute(file, "Comment", comment.toLatin1().constData());

		foreach(QObject* o, /*RtObject::root()->*/children())
		{
			if (RtObject* rto = qobject_cast<RtObject*>(o))
			{
				RtSession* ss = qobject_cast<RtSession*>(rto);
				if (!ss) writeObject(file, rto);
			}
		}

    }  // end of try block

    // catch failure caused by the H5File operations
    catch( FileIException error )
    {
		S = "HDF5 File Error.";
		S += " In function ";
		S += error.getCFuncName();
		S += ". ";
		S += error.getCDetailMsg();
    }

    // catch failure caused by the DataSet operations
    catch( DataSetIException error )
    {
		S = "HDF5 DataSet Error.";
		S += " In function ";
		S += error.getCFuncName();
		S += ". ";
		S += error.getCDetailMsg();
    }

    // catch failure caused by the DataSpace operations
    catch( DataSpaceIException error )
    {
		S = "HDF5 DataSpace Error.";
		S += " In function ";
		S += error.getCFuncName();
		S += ". ";
		S += error.getCDetailMsg();
    }

    // catch failure caused by the Attribute operations
    catch( AttributeIException error )
    {
		S = "HDF5 Attribute Error.";
		S += " In function ";
		S += error.getCFuncName();
		S += ". ";
		S += error.getCDetailMsg();
    }

    // catch failure caused by the DataType operations
    catch( DataTypeIException error )
    {
		S = "HDF5 DataType Error.";
		S += " In function ";
		S += error.getCFuncName();
		S += ". ";
		S += error.getCDetailMsg();
    }

	if (file) delete file;

	return S;
}

void writeProperties(H5Object* h5obj, const RtObject* m_object, const QMetaObject* metaObject);

void writeObject(H5File* file, RtObject* obj)
{
	/*
	 * Create a group in the file
	 */
	QString name = obj->fullName();
	name.replace('.','/');
	name.prepend('/');

	RtVector* vec = qobject_cast<RtVector*>(obj);
	if (vec)
	{
		hsize_t len = vec->data().size();
		if (len<1) len=1;

		DataSpace* dspace = new DataSpace(1,&len);
		DataSet* ds = new DataSet( file->createDataSet( name.toLatin1(),
			PredType::NATIVE_DOUBLE, *dspace) );

		writeProperties(ds,obj,obj->metaObject());

		if (vec->data().size())
			ds->write(vec->data_ptr(), PredType::NATIVE_DOUBLE);


		delete ds;
		delete dspace;
	}
	else
	{
		Group* group = new Group( file->createGroup( name.toLatin1() ));

		writeProperties(group,obj,obj->metaObject());

		foreach(QObject* o, obj->children())
		{
			if (RtObject* rto = qobject_cast<RtObject*>(o))
				writeObject(file, rto);
		}

		delete group;
	}

}

void writeStringAttribute(H5Object* h5obj, const char* name, const char* value)
{
	int len = strlen(value);
	if (len==0) return;
	StrType type(PredType::C_S1, len);
	hsize_t dims = 1;
	DataSpace space(1,&dims);
	Attribute* attr = new Attribute( h5obj->createAttribute(name, type, space) );
	attr->write(type, value);
	delete attr;
}
void writeFileAttribute(H5File* h5obj, const char* name, const char* value)
{
	int len = strlen(value);
	if (len==0) return;
	StrType type(PredType::C_S1, len);
	hsize_t dims = 1;
	DataSpace space(1,&dims);
	DataSet* ds = new DataSet( h5obj->createDataSet(name, type, space) );
	ds->write(value,type);
	delete ds;
}

void writeProperties(H5Object* h5obj, const RtObject* m_object, const QMetaObject* metaObject)
{
	// get the super-class meta-object
	const QMetaObject* super = metaObject->superClass();

	// if it is null -> we reached the top, i.e. the QObject level -> return
	if (!super) return;

	// if this is the first , write the class name
	if (metaObject==m_object->metaObject())
	{
		writeStringAttribute(h5obj,"Class", metaObject->className());
	}

	// first write the properties of the super-class (this produces all properties up to the current object)
	writeProperties(h5obj, m_object, super);

	// write this class properties
	for (int idx = metaObject->propertyOffset(); idx < metaObject->propertyCount(); idx++) 
	{
		QMetaProperty metaProperty = metaObject->property(idx);
		if (metaProperty.isReadable())
		{
			QVariant value = metaProperty.read(m_object);
			if (metaProperty.isEnumType())
			{
				QMetaEnum metaEnum = metaProperty.enumerator();
				int i = *reinterpret_cast<const int *>(value.constData());
				writeStringAttribute(h5obj,metaProperty.name(),metaEnum.valueToKey(i));
			}
			else {
				int objtype = value.userType();
				if (objtype==QVariant::Bool || objtype==QVariant::Char ||
					objtype==QVariant::Int || objtype==QVariant::UInt)
				{
					DataSpace space(H5S_SCALAR);
					Attribute* attr = new Attribute(
						h5obj->createAttribute(metaProperty.name(),
						PredType::NATIVE_INT, space)
						);
					int i = value.toInt();
					attr->write(PredType::NATIVE_INT,&i);
					delete attr;
				}
				else if (objtype==QVariant::String)
				{
					QString S = value.toString();
					if (S.isEmpty()) S=" ";
					writeStringAttribute(h5obj,metaProperty.name(),S.toLatin1().constData());
				}
				else if (objtype==QVariant::Double)
				{
					DataSpace space(H5S_SCALAR);
					Attribute* attr = new Attribute(
						h5obj->createAttribute(metaProperty.name(),
						PredType::NATIVE_DOUBLE, space)
						);
					double d = value.toDouble();
					attr->write(PredType::NATIVE_DOUBLE,&d);
					delete attr;
				}
				else if (objtype==qMetaTypeId<RtIntVector>())
				{
					RtIntVector v = value.value<RtIntVector>();
					hsize_t sz = v.size();
					if (sz<1) sz=1;
					ArrayType type(PredType::NATIVE_INT, 1, &sz);
					hsize_t dims = 1;
					DataSpace space(1,&dims);
					Attribute* attr = new Attribute(
						h5obj->createAttribute(metaProperty.name(),
						type, space)
						);
					if (v.size())
						attr->write(type, v.constData());
					delete attr;
				}
				else if (objtype==qMetaTypeId<RtUintVector>())
				{
					RtUintVector v = value.value<RtUintVector>();
					hsize_t sz = v.size();
					if (sz<1) sz=1;
					ArrayType type(PredType::NATIVE_UINT, 1, &sz);
					hsize_t dims = 1;
					DataSpace space(1,&dims);
					Attribute* attr = new Attribute(
						h5obj->createAttribute(metaProperty.name(),
						type, space)
						);
					if (v.size())
						attr->write(type, v.constData());
					delete attr;
				}
				else if (objtype==qMetaTypeId<RtDoubleVector>())
				{
					RtDoubleVector v = value.value<RtDoubleVector>();
					hsize_t sz = v.size();
					if (sz<1) sz=1;
					ArrayType type(PredType::NATIVE_DOUBLE, 1, &sz);
					hsize_t dims = 1;
					DataSpace space(1,&dims);
					Attribute* attr = new Attribute(
						h5obj->createAttribute(metaProperty.name(),
						type, space)
						);
					if (v.size())
						attr->write(type, v.constData());
					delete attr;
				}
			}
		}
	}

    // write the object's dynamic properties
    if (metaObject==m_object->metaObject()) {
        if (!m_object->dynamicPropertyNames().isEmpty()) {
            foreach(const QByteArray& ba, m_object->dynamicPropertyNames())
            {
                QVariant v = m_object->property(ba.constData());
                writeStringAttribute(h5obj,ba.constData(),v.toString().toLatin1());
            }
        }
    }
}



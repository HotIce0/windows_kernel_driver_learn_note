#include "Driver.h"


#pragma PAGEDCODE
VOID MyDriverUnload(IN PDRIVER_OBJECT pDriverObject)
{
	KdPrint(("Enter MyDriverUnload\n"));
	PDEVICE_OBJECT pNextDevObj = NULL;

	pNextDevObj = pDriverObject->DeviceObject;
	while (pNextDevObj != NULL) {
		PDEVICE_EXTENSION pDevExt = (PDEVICE_EXTENSION)pNextDevObj->DeviceExtension;

		IoDeleteSymbolicLink(&pDevExt->ustrSymLinkName);
		pNextDevObj = pNextDevObj->NextDevice;
		IoDeleteDevice(pDevExt->pDevice);
	}
}

NTSTATUS MyDriverDispatchRoutine(IN PDEVICE_OBJECT pDevObject, IN PIRP pIrp)
{
	KdPrint(("Enter MyDriverDispatchRoutine\n"));
	NTSTATUS status = STATUS_SUCCESS;
	
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;	// bytes xfered
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	KdPrint(("End MyDriver DispatchRoutine\n"));
	return status;
}

#pragma INITCODE

NTSTATUS CreateDevice(IN PDRIVER_OBJECT pDriverObject)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PDEVICE_OBJECT pDevObj = NULL;
	PDEVICE_EXTENSION pDevExt = NULL;

	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, L"\\Deivce\\MyDevice");
	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&symLinkName, L"\\??\\MyDev");

	status = IoCreateDevice(pDriverObject,
		sizeof(DEVICE_EXTENSION),
		&devName,
		FILE_DEVICE_UNKNOWN,
		0, TRUE,
		&pDevObj);
	if (!NT_SUCCESS(status)) {
		KdPrint(("IoCreateDevice Failed with %d\n", status));
		return status;
	}

	pDevObj->Flags |= DO_BUFFERED_IO;
	pDevExt = (PDEVICE_EXTENSION)pDevObj->DeviceExtension;

	pDevExt->pDevice = pDevObj;
	pDevExt->ustrDeviceName = devName;
	pDevExt->ustrSymLinkName = symLinkName;

	status = IoCreateSymbolicLink(&symLinkName, &devName);
	if (!NT_SUCCESS(status)) {
		IoDeleteDevice(pDevObj);
		KdPrint(("IoCreateSymbolinkLink failed with %d\n", status));
		return status;
	}

	return STATUS_SUCCESS;
}

extern "C" NTSTATUS DriverEntry(
	IN PDRIVER_OBJECT pDriverObject,
	IN PUNICODE_STRING pRegistryPath)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	KdPrint(("Enter DriverEntry\n"));

	pDriverObject->DriverUnload = MyDriverUnload;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = MyDriverDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = MyDriverDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_READ] = MyDriverDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = MyDriverDispatchRoutine;

	status = CreateDevice(pDriverObject);
	
	KdPrint(("End DriverEntry\n"));

	return STATUS_SUCCESS;
}


